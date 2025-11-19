package cluster_top

import (
	"context"
	"time"

	"github.com/yandex/perforator/perforator/pkg/storage/cluster_top/aggregated"
)

type TimeRange struct {
	From time.Time
	To   time.Time
}

type ServiceProcessingHandler interface {
	GetServiceName() string

	GetGeneration() int

	GetTimeRange() TimeRange

	Finalize(ctx context.Context, processingErr error)
}

type ServiceSelector interface {
	SelectService(ctx context.Context, heavy bool) (ServiceProcessingHandler, error)
}

type Function = aggregated.Function

type ServicePerfTop = aggregated.ServicePerfTop

type ClusterPerfTopAggregator interface {
	Save(ctx context.Context, servicePerfTop *ServicePerfTop) error

	Print(ctx context.Context) error
}
