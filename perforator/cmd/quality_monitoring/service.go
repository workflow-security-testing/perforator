package main

import (
	"context"
	"fmt"
	"net"
	"net/http"
	"sync"
	"time"

	"golang.org/x/sync/errgroup"
	"google.golang.org/protobuf/types/known/timestamppb"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/metrics"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
	"github.com/yandex/perforator/perforator/pkg/tracing"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	"github.com/yandex/perforator/perforator/proto/lib/time_interval"
	"github.com/yandex/perforator/perforator/proto/perforator"
	"github.com/yandex/perforator/perforator/symbolizer/pkg/client"
)

type serviceMetrics struct {
	framesCount               metrics.Counter
	samplesCount              metrics.Counter
	unsymbolizedCount         metrics.Counter
	profilesCount             metrics.Counter
	mergeTimer                metrics.Timer
	mergeRequestsSuccess      metrics.Counter
	mergeRequestsFail         metrics.Counter
	mergeRequestsNoStatistics metrics.Counter
}

func newServiceMetrics(reg xmetrics.Registry, service string) serviceMetrics {
	tags := map[string]string{"user_service": service}
	return serviceMetrics{
		framesCount:               reg.WithTags(tags).Counter("frames.count"),
		samplesCount:              reg.WithTags(tags).Counter("samples.count"),
		unsymbolizedCount:         reg.WithTags(tags).Counter("frames.unsymbolized.count"),
		profilesCount:             reg.WithTags(tags).Counter("profiles.count"),
		mergeTimer:                reg.WithTags(tags).Timer("profile.merge"),
		mergeRequestsSuccess:      reg.WithTags(map[string]string{"user_service": service, "status": "success"}).Counter("requests.merge_profiles"),
		mergeRequestsFail:         reg.WithTags(map[string]string{"user_service": service, "status": "fail"}).Counter("requests.merge_profiles"),
		mergeRequestsNoStatistics: reg.WithTags(tags).Counter("requests.merge_profiles.no_statistics"),
	}
}

func (m *serviceMetrics) recordStats(stats *perforator.ProfileStatistics) {
	m.framesCount.Add(int64(stats.TotalFrameCount))
	m.samplesCount.Add(int64(stats.UniqueSampleCount))
	m.unsymbolizedCount.Add(int64(stats.UnsymbolizedFrameCount))
	m.profilesCount.Inc()
}

type MonitoringService struct {
	cfg         *Config
	reg         xmetrics.Registry
	proxyClient *client.Client
	uiURLPrefix string
	total       serviceMetrics
}

func NewMonitoringService(
	ctx context.Context,
	cfg *Config,
	logger xlog.Logger,
	reg xmetrics.Registry,
) (*MonitoringService, error) {
	host, _, err := net.SplitHostPort(cfg.Client.URL)
	if err != nil {
		return nil, fmt.Errorf("failed to get perforator host: %w", err)
	}

	exporter, err := tracing.NewExporter(ctx, cfg.Tracing)
	if err != nil {
		return nil, fmt.Errorf("failed to initialize tracing span exporter: %w", err)
	}

	shutdown, _, err := tracing.Initialize(ctx, logger.WithName("tracing").Logger(), exporter, "perforator", "monitoring")
	if err != nil {
		return nil, fmt.Errorf("failed to initialize tracing: %w", err)
	}
	defer func() {
		if err != nil && shutdown != nil {
			_ = shutdown(ctx)
		}
	}()
	logger.Info(ctx, "Successfully initialized tracing")

	c, err := client.NewClient(ctx, &cfg.Client, logger.WithName("client"))
	if err != nil {
		return nil, fmt.Errorf("failed to initialize perforator client: %w", err)
	}
	logger.Info(ctx, "Created perforator client")

	return &MonitoringService{
		cfg:         cfg,
		proxyClient: c,
		reg:         reg,
		uiURLPrefix: host + "/task/",
		total:       newServiceMetrics(reg, "all"),
	}, nil
}

func (s *MonitoringService) gatherServicesMetrics(ctx context.Context, logger xlog.Logger) error {
	listServicesCtx, cancel := context.WithTimeout(ctx, s.cfg.Timeout)
	defer cancel()
	services, err := s.proxyClient.ListServices(listServicesCtx, s.cfg.ServicesOffset, s.cfg.ServicesNumberToCheck, nil, nil, "profiles")
	if err != nil {
		logger.Error(ctx, "failed to list services", log.Error(err))
		return err
	}

	logger.Debug(ctx, "Number of services", log.Int("number of services", len(services)))

	var wg sync.WaitGroup
	servicesCh := make(chan *perforator.ServiceMeta)

	for i := 0; i < s.cfg.ServicesCheckingConcurrency; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for service := range servicesCh {
				if err := s.gatherServiceProfilesMetrics(ctx, logger, service.ServiceID); err != nil {
					logger.Error(ctx, "Failed to gather metrics", log.Error(err), log.String("service_id", service.ServiceID))
				}
			}
		}()
	}

	for _, service := range services {
		servicesCh <- service
	}
	close(servicesCh)

	wg.Wait()
	return nil
}

func (s *MonitoringService) gatherServiceProfilesMetrics(ctx context.Context, logger xlog.Logger, service string) error {
	ctx, cancel := context.WithTimeout(ctx, s.cfg.Timeout)
	defer cancel()
	logger = logger.With(log.String("service_id", service))
	m := newServiceMetrics(s.reg, service)

	toTS := time.Now()
	fromTS := toTS.Add(-s.cfg.CheckQualityInterval)

	selector, err := profilequerylang.SelectorToString(
		profilequerylang.NewBuilder().From(fromTS).To(toTS).Services(service).Build(),
	)
	if err != nil {
		logger.Error(ctx, "Failed to create selector", log.Error(err))
		return err
	}

	start := time.Now()
	taskId, res, err := s.proxyClient.MergeProfilesProto(
		ctx,
		&perforator.MergeProfilesRequest{
			Query: &perforator.ProfileQuery{
				Selector: selector,
				TimeInterval: &time_interval.TimeInterval{
					From: timestamppb.New(fromTS),
					To:   timestamppb.New(toTS),
				},
			},
			MaxSamples: s.cfg.MaxSamplesToMerge,
			Format: &perforator.RenderFormat{
				Format: &perforator.RenderFormat_RawProfile{
					RawProfile: &perforator.RawProfileOptions{},
				},
			},
		},
		"request by quality-monitoring",
	)
	duration := time.Since(start)
	m.mergeTimer.RecordDuration(duration)
	s.total.mergeTimer.RecordDuration(duration)

	if err != nil {
		m.mergeRequestsFail.Inc()
		s.total.mergeRequestsFail.Inc()
		logger.Error(ctx, "Failed to merge profiles", log.Error(err))
		return err
	}

	m.mergeRequestsSuccess.Inc()
	s.total.mergeRequestsSuccess.Inc()

	if len(res.ProfileMeta) == 0 {
		logger.Warn(ctx, "No profiles to merge")
		return nil
	}

	stats := res.Statistics
	if stats == nil {
		m.mergeRequestsNoStatistics.Inc()
		s.total.mergeRequestsNoStatistics.Inc()
		logger.Error(ctx, "No statistics in response")
		return fmt.Errorf("no statistics in response")
	}

	m.recordStats(stats)
	s.total.recordStats(stats)

	unsymbolizedPercent := float64(0)
	if stats.TotalFrameCount > 0 {
		unsymbolizedPercent = float64(stats.UnsymbolizedFrameCount) / float64(stats.TotalFrameCount) * 100
	}

	if stats.UnsymbolizedFrameCount == 0 {
		logger.Info(ctx, "Profile fully symbolized",
			log.String("url", s.uiURLPrefix+taskId),
		)
	} else {
		logger.Warn(ctx, "Profile has unsymbolized frames",
			log.String("url", s.uiURLPrefix+taskId),
			log.String("unsymbolized_percent", fmt.Sprintf("%.2f", unsymbolizedPercent)),
		)
	}

	return nil
}

func (s *MonitoringService) Run(ctx context.Context, logger xlog.Logger, metricsPort uint) error {
	g, ctx := errgroup.WithContext(ctx)

	g.Go(func() error {
		logger.Info(ctx, "Starting metrics server", log.UInt("port", metricsPort))
		http.Handle("/metrics", s.reg.HTTPHandler(ctx, logger))
		return http.ListenAndServe(fmt.Sprintf(":%d", metricsPort), nil)
	})

	g.Go(func() error {
		ticker := time.NewTicker(s.cfg.IterationSplay)
		defer ticker.Stop()

		for {
			logger.Info(ctx, "Starting iteration")
			if err := s.gatherServicesMetrics(ctx, logger); err != nil {
				logger.Error(ctx, "Failed to gather services metrics", log.Error(err))
				time.Sleep(s.cfg.SleepAfterFailedServicesChecking)
				continue
			}
			logger.Info(ctx, "Finished iteration")

			select {
			case <-ctx.Done():
				return ctx.Err()
			case <-ticker.C:
			}
		}
	})

	return g.Wait()
}
