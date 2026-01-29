package main

import (
	"context"
	"flag"
	standardLog "log"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/internal/symbolizer/quality_monitoring/internal/config"
	"github.com/yandex/perforator/perforator/internal/symbolizer/quality_monitoring/internal/service"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

func main() {
	configPath := flag.String("config", "", "Path to monitoring service config")
	logLevel := flag.String("log-level", "info", "Logging level - ('info') {'debug', 'info', 'warn', 'error'}")
	metricsPort := flag.Uint("metrics-port", 85, "Port on which the metrics server is running")

	flag.Parse()

	reg := xmetrics.NewRegistry()

	logger, stopLogger, err := setupLogger(*logLevel, reg)
	if err != nil {
		standardLog.Fatalf("can't create logger: %s", err)
	}
	defer stopLogger()
	ctx := context.Background()

	cfg, err := config.LoadConfig(*configPath)
	if err != nil {
		standardLog.Fatalf("can't load config: %s", err)
	}

	serv, err := service.NewMonitoringService(ctx, cfg, logger, reg)
	if err != nil {
		standardLog.Fatalf("can't create monitoring server: %s", err)
	}

	err = serv.Run(
		ctx,
		logger,
		&service.RunConfig{
			MetricsPort: *metricsPort,
		})
	if err != nil {
		logger.Error(ctx, "Service is stopping", log.Error(err))
	}
}

func setupLogger(logLevel string, reg xmetrics.Registry) (xlog.Logger, func(), error) {
	level, err := log.ParseLevel(logLevel)
	if err != nil {
		return nil, nil, err
	}

	return xlog.ForDaemon(xlog.DaemonConfig{
		Level: level,
	}, reg)
}
