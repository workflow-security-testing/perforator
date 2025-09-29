package factory

import (
	"errors"
	"fmt"

	hasql "golang.yandex/hasql/sqlx"

	"github.com/yandex/perforator/perforator/pkg/storage/custom_profiling_operation"
	"github.com/yandex/perforator/perforator/pkg/storage/custom_profiling_operation/postgres"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type options struct {
	postgresCluster *hasql.Cluster
}

type Option = func(*options)

func defaultOpts() *options {
	return &options{}
}

func WithPostgresCluster(cluster *hasql.Cluster) Option {
	return func(o *options) {
		o.postgresCluster = cluster
	}
}

func NewStorage(
	logger xlog.Logger,
	storageType custom_profiling_operation.CustomProfilingOperationStorageType,
	optAppliers ...Option,
) (custom_profiling_operation.Storage, error) {
	options := defaultOpts()
	for _, optApplier := range optAppliers {
		optApplier(options)
	}

	switch storageType {
	case custom_profiling_operation.Postgres:
		if options.postgresCluster == nil {
			return nil, errors.New("postgres cluster is not specified")
		}

		return postgres.NewStorage(logger, options.postgresCluster), nil
	default:
		return nil, fmt.Errorf("unknown storage type: %s", storageType)
	}
}
