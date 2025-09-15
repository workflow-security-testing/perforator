package profile_event

import (
	"context"
	"sync"
	"time"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/metrics"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/kafka/producer"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type Config struct {
	QueueSize     int `yaml:"queue_size"`
	WorkersNumber int `yaml:"workers_number"`
}

func (c *Config) fillDefaults() {
	if c.QueueSize <= 0 {
		c.QueueSize = 1000
	}
	if c.WorkersNumber <= 0 {
		c.WorkersNumber = 5
	}
}

type AsyncSignalProfileEventPublisher struct {
	m        sync.RWMutex
	closed   bool
	cfg      Config
	producer *producer.JSONProducer[SignalProfileEvent]
	logger   xlog.Logger
	metrics  *publisherMetrics
	reg      xmetrics.Registry

	messageChan chan SignalProfileMessage
}

type publisherMetrics struct {
	messagesSent             metrics.Counter
	messagesSentErrors       metrics.Counter
	messagesDropped          metrics.Counter
	messageQueueLength       metrics.Gauge
	messagesSentSuccessTimer metrics.Timer
	messagesSentFailTimer    metrics.Timer
}

func newPublisherMetrics(reg xmetrics.Registry) *publisherMetrics {
	return &publisherMetrics{
		messagesSent:             reg.WithTags(map[string]string{"kind": "sent"}).Counter("messages.count"),
		messagesDropped:          reg.WithTags(map[string]string{"kind": "dropped"}).Counter("messages.count"),
		messagesSentSuccessTimer: reg.WithTags(map[string]string{"kind": "sent"}).Timer("messages.timer"),
		messagesSentFailTimer:    reg.WithTags(map[string]string{"kind": "dropped"}).Timer("messages.timer"),
		messagesSentErrors:       reg.Counter("publish_errors.count"),
		messageQueueLength:       reg.Gauge("message_queue_length.gauge"),
	}
}

func NewAsyncSignalProfileEventPublisher(
	pr producer.Producer,
	logger xlog.Logger,
	reg xmetrics.Registry,
	cfg Config,
) *AsyncSignalProfileEventPublisher {
	cfg.fillDefaults()

	return &AsyncSignalProfileEventPublisher{
		cfg:         cfg,
		producer:    producer.NewJSONProducer[SignalProfileEvent](pr),
		logger:      logger,
		metrics:     newPublisherMetrics(reg),
		reg:         reg,
		messageChan: make(chan SignalProfileMessage, cfg.QueueSize),
	}
}

func (p *AsyncSignalProfileEventPublisher) processMessages(ctx context.Context) {
	for {
		select {
		case <-ctx.Done():
			return
		case msg, ok := <-p.messageChan:
			if !ok {
				return
			}
			p.metrics.messageQueueLength.Set(float64(len(p.messageChan)))
			p.publish(ctx, msg)
		}
	}
}

func (p *AsyncSignalProfileEventPublisher) publish(ctx context.Context, msg SignalProfileMessage) {
	start := time.Now()
	var err error
	defer func() {
		if err != nil {
			p.metrics.messagesSentFailTimer.RecordDuration(time.Since(start))
			p.metrics.messagesSentErrors.Inc()
		} else {
			p.metrics.messagesSentSuccessTimer.RecordDuration(time.Since(start))
			p.metrics.messagesSent.Inc()
		}
	}()

	if err := p.producer.Publish(ctx, []byte(msg.partitionKey), msg.event); err != nil {
		p.logger.Error(
			ctx,
			"Failed to publish profile event)",
			log.Error(err),
			log.String("key", msg.partitionKey),
			log.String("profile_id", msg.event.ProfileID),
		)
		return
	}
}

func (p *AsyncSignalProfileEventPublisher) TryEnqueueForPublish(ctx context.Context, ev *SignalProfileEvent) {
	p.m.RLock()
	defer p.m.RUnlock()
	if p.closed {
		p.logger.Error(
			ctx,
			"Failed to enqueue: publisher is closed",
		)
		return
	}

	key := ev.Service
	select {
	case <-ctx.Done():
		return
	case p.messageChan <- SignalProfileMessage{
		partitionKey: key,
		event:        ev,
	}:
		p.metrics.messageQueueLength.Set(float64(len(p.messageChan)))
	default:
		// queue full -> drop
		p.metrics.messagesDropped.Inc()
		p.logger.Warn(
			ctx,
			"Profile event dropped (queue full)",
			log.String("key", key),
			log.String("profile_id", ev.ProfileID),
		)
	}
}

func (p *AsyncSignalProfileEventPublisher) Run(ctx context.Context) error {
	var wg sync.WaitGroup

	for i := 0; i < p.cfg.WorkersNumber; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			p.processMessages(ctx)
		}()
	}

	<-ctx.Done()

	p.m.Lock()
	p.closed = true
	close(p.messageChan)
	p.m.Unlock()

	// wait for workers
	wg.Wait()

	if err := p.producer.Close(); err != nil {
		return err
	}

	return ctx.Err()
}
