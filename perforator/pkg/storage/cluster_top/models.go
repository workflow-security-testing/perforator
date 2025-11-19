package clustertop

import (
	"context"

	"github.com/yandex/perforator/perforator/pkg/storage/cluster_top/aggregated"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

type GenerationsStorageType string

const (
	Postgres GenerationsStorageType = "postgres"
)

type AggregationStorageType string

const (
	Clickhouse AggregationStorageType = "clickhouse"
)

type Config struct {
	GenerationsStorage GenerationsStorageType `yaml:"generations_storage"`
	AggregationStorage AggregationStorageType `yaml:"aggregation_storage"`
}

type Storage interface {
	ListGenerations(ctx context.Context) ([]*perforator.ClusterTopGeneration, error)
	AggregateClusterTop(ctx context.Context, generation uint32, query string, aggregationType aggregated.GroupByMode) (*perforator.ClusterTopResponse, error)
	SaveClusterTopEntry(ctx context.Context, servicePerfTop *aggregated.ServicePerfTop) error
}
