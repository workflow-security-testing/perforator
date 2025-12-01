package aggregated

import (
	"context"
	"math/big"

	"github.com/yandex/perforator/perforator/pkg/storage/util"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

type GroupByMode string

const (
	GroupByFunction GroupByMode = "function"
	GroupByService  GroupByMode = "service"
)

type MatchMode string

const (
	ExactMatch     MatchMode = "exact"
	RegexMatch     MatchMode = "regex"
	SubstringMatch MatchMode = "substr"
)

type Filter struct {
	FunctionFilter string
	ServiceFilter  string
	// Controls FunctionFilter mode
	// for ServiceFilter always exact match
	FunctionFilterMatchMode MatchMode
}

type AggregationStorage interface {
	SaveClusterTopEntry(ctx context.Context, servicePerfTop *ServicePerfTop) error
	AggregateClusterTop(ctx context.Context, generation uint32, filter *Filter, aggregationType GroupByMode, pagination util.Pagination) ([]*perforator.ClusterTopEntry, error)
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
