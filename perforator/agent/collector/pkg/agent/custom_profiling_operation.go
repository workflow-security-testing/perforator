package agent

import (
	"context"
	"errors"
	"os"

	"golang.org/x/sync/errgroup"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/internal/agent_gateway/client/custom_profiling_operation"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	cpo_proto "github.com/yandex/perforator/perforator/proto/custom_profiling_operation"
)

// CPO is a short name for Custom Profiling Operation.

type CPOProcessorConfig struct {
	PolledOperationsQueueSize uint64 `yaml:"polled_operations_queue_size"`
	Host                      string `yaml:"host"`
}

func (c *CPOProcessorConfig) FillDefault() {
	if c.PolledOperationsQueueSize == 0 {
		c.PolledOperationsQueueSize = 100
	}
}

type cpoProcessor struct {
	l                     xlog.Logger
	config                *CPOProcessorConfig
	cpoClient             *custom_profiling_operation.Client
	polledOperationsQueue chan *cpo_proto.Operation
	handler               *cpoHandler
}

func newCPOProcessor(
	l xlog.Logger,
	config *CPOProcessorConfig,
	cpoClient *custom_profiling_operation.Client,
) (*cpoProcessor, error) {
	if config.Host == "" {
		var err error
		config.Host, err = os.Hostname()
		if err != nil {
			return nil, err
		}
	}

	return &cpoProcessor{
		l:                     l,
		config:                config,
		cpoClient:             cpoClient,
		polledOperationsQueue: make(chan *cpo_proto.Operation, config.PolledOperationsQueueSize),
		handler:               newCPOHandler(l),
	}, nil
}

func (p *cpoProcessor) runPoller(ctx context.Context) error {
	defer close(p.polledOperationsQueue)

	poller := p.cpoClient.CreateLongPoller()
	for {
		operations, err := poller.PollOperations(ctx, p.config.Host, nil)
		if err != nil {
			return err
		}

		for _, operation := range operations {
			p.polledOperationsQueue <- operation
		}
	}
}

func (p *cpoProcessor) runConsumer(ctx context.Context) error {
	for operation := range p.polledOperationsQueue {
		err := p.handler.handle(ctx, operation)
		if err != nil {
			p.l.Error(ctx, "Failed to handle operation", log.Error(err))
		}
	}

	return nil
}

func (p *cpoProcessor) Run(ctx context.Context) error {
	g, ctx := errgroup.WithContext(ctx)

	g.Go(func() error {
		return p.runPoller(ctx)
	})

	g.Go(func() error {
		return p.runConsumer(ctx)
	})

	return g.Wait()
}

type cpoHandler struct {
	l xlog.Logger
}

func newCPOHandler(l xlog.Logger) *cpoHandler {
	return &cpoHandler{
		l: l,
	}
}

func (h *cpoHandler) handle(ctx context.Context, operation *cpo_proto.Operation) error {
	// TODO: implement
	return errors.New("custom profiling operation handler is not implemented yet")
}
