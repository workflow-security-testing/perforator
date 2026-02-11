package testutils

import (
	"context"
	"errors"
	"fmt"

	"github.com/golang-migrate/migrate/v4"
	dPgx "github.com/golang-migrate/migrate/v4/database/pgx"
	_ "github.com/golang-migrate/migrate/v4/source/file"
	_ "github.com/jackc/pgx/v5/stdlib"
	hasql "golang.yandex/hasql/sqlx"

	"github.com/yandex/perforator/perforator/pkg/postgres"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type clusterOptions struct {
	migrationsDir string
}

type ClusterOption func(*clusterOptions)

func WithMigrations(dir string) ClusterOption {
	return func(o *clusterOptions) {
		o.migrationsDir = dir
	}
}

func NewTestCluster(pingCtx context.Context, l xlog.Logger, opts ...ClusterOption) (*hasql.Cluster, error) {
	options := clusterOptions{}
	for _, opt := range opts {
		opt(&options)
	}

	cfg, err := DefaultTestConfig()
	if err != nil {
		return nil, err
	}

	cluster, err := postgres.NewCluster(pingCtx, pingCtx, l, "test", &cfg)
	if err != nil {
		return nil, err
	}

	if options.migrationsDir != "" {
		err = runMigrationsOnCluster(pingCtx, cluster, options.migrationsDir)
		if err != nil {
			closeErr := cluster.Close()
			return nil, errors.Join(err, closeErr)
		}
	}

	return cluster, nil
}

func runMigrationsOnCluster(ctx context.Context, cluster *hasql.Cluster, migrationsDir string) error {
	primary, err := cluster.WaitForPrimary(ctx)
	if err != nil {
		return fmt.Errorf("failed to wait for primary for migrations: %w", err)
	}

	driver, err := dPgx.WithInstance(primary.DB(), &dPgx.Config{})
	if err != nil {
		return fmt.Errorf("failed to create migration driver: %w", err)
	}

	m, err := migrate.NewWithDatabaseInstance("file://"+migrationsDir, "pgx", driver)
	if err != nil {
		return fmt.Errorf("failed to create migration instance: %w", err)
	}

	err = m.Up()
	if err != nil && err != migrate.ErrNoChange {
		return fmt.Errorf("failed to run migrations: %w", err)
	}

	return nil
}
