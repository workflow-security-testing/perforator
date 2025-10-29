package custom_profiling_operation

import (
	"context"

	"google.golang.org/protobuf/types/known/timestamppb"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/agent/collector/pkg/agent/custom_profiling_operation/models"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	cpo_proto "github.com/yandex/perforator/perforator/proto/custom_profiling_operation"
)

var (
	_ models.Handler = (*handler)(nil)
)

type handler struct {
	l        xlog.Logger
	registry models.OperationExecutionRegistry
	reporter models.OperationReporter
}

func NewHandler(l xlog.Logger, registry models.OperationExecutionRegistry, reporter models.OperationReporter) *handler {
	return &handler{
		l:        l,
		registry: registry,
		reporter: reporter,
	}
}

func (h *handler) tryFailOperation(ctx context.Context, operation *cpo_proto.Operation, operationError error) {
	err := h.reporter.UpdateOperationStatus(ctx, operation.ID, &models.OperationStatus{
		State:     cpo_proto.OperationState_Failed,
		Timestamp: timestamppb.Now(),
		Error:     operationError.Error(),
	})
	if err != nil {
		h.l.Error(ctx, "Failed to report operation status", log.Error(err))
	}
}

func (h *handler) Handle(ctx context.Context, operation *cpo_proto.Operation) error {
	cancelExecutionCtx, err := h.registry.Ensure(ctx, operation)
	if err != nil {
		go func() {
			h.tryFailOperation(ctx, operation, err)
		}()
		return err
	}

	if operation.TargetState != nil && operation.TargetState.State == cpo_proto.OperationState_Stopped {
		cancelExecutionCtx()
	}

	return nil
}
