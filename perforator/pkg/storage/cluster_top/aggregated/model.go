package aggregated

import (
	"context"
	"math/big"

	"github.com/yandex/perforator/perforator/pkg/storage/util"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

const (
	GroupByFunction GroupByMode = "function"
	GroupByService  GroupByMode = "service"
)

type AggregationStorage interface {
	SaveClusterTopEntry(ctx context.Context, servicePerfTop *ServicePerfTop) error
	AggregateClusterTop(ctx context.Context, generation uint32, pattern string, aggregationType GroupByMode, pagination util.Pagination) ([]*perforator.ClusterTopEntry, error)
}

type Function struct {
	Name             string
	SelfCycles       big.Int
	CumulativeCycles big.Int
}

type ServicePerfTop struct {
	Generation  int
	ServiceName string

	Functions []Function
}
