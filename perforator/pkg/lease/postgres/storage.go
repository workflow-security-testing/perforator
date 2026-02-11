package postgres

import (
	"context"
	"fmt"
	"time"

	"github.com/Masterminds/squirrel"
	hasql "golang.yandex/hasql/sqlx"

	"github.com/yandex/perforator/perforator/pkg/xlog"
)

const (
	leasesTable = "leases"

	acquireOnConflictSuffix = "ON CONFLICT (name) DO UPDATE SET holder = EXCLUDED.holder, expires_at = EXCLUDED.expires_at WHERE " + leasesTable + ".expires_at < NOW()"
)

var (
	psql = squirrel.StatementBuilder.PlaceholderFormat(squirrel.Dollar)
)

type Storage struct {
	logger  xlog.Logger
	cluster *hasql.Cluster
}

func NewStorage(logger xlog.Logger, cluster *hasql.Cluster) *Storage {
	return &Storage{
		logger:  logger.WithName("LeaseStorage"),
		cluster: cluster,
	}
}

func (s *Storage) Acquire(ctx context.Context, name, holder string, ttl time.Duration) (bool, error) {
	primary, err := s.cluster.WaitForPrimary(ctx)
	if err != nil {
		return false, fmt.Errorf("failed to wait for primary: %w", err)
	}

	expiresAt := time.Now().Add(ttl)

	query, args, err := psql.Insert(leasesTable).
		Columns("name", "holder", "expires_at").
		Values(name, holder, expiresAt).
		Suffix(acquireOnConflictSuffix).
		ToSql()
	if err != nil {
		return false, fmt.Errorf("failed to build query: %w", err)
	}

	res, err := primary.DBx().ExecContext(ctx, query, args...)
	if err != nil {
		return false, fmt.Errorf("failed to acquire lease: %w", err)
	}

	rows, err := res.RowsAffected()
	if err != nil {
		return false, err
	}

	return rows > 0, nil
}

func (s *Storage) Renew(ctx context.Context, name, holder string, ttl time.Duration) (bool, error) {
	primary, err := s.cluster.WaitForPrimary(ctx)
	if err != nil {
		return false, fmt.Errorf("failed to wait for primary: %w", err)
	}

	expiresAt := time.Now().Add(ttl)

	query, args, err := psql.Update(leasesTable).
		Set("expires_at", expiresAt).
		Where(squirrel.Eq{"name": name, "holder": holder}).
		ToSql()
	if err != nil {
		return false, fmt.Errorf("failed to build query: %w", err)
	}

	res, err := primary.DBx().ExecContext(ctx, query, args...)
	if err != nil {
		return false, fmt.Errorf("failed to renew lease: %w", err)
	}

	rows, err := res.RowsAffected()
	if err != nil {
		return false, err
	}

	return rows > 0, nil
}

func (s *Storage) Release(ctx context.Context, name, holder string) error {
	primary, err := s.cluster.WaitForPrimary(ctx)
	if err != nil {
		return fmt.Errorf("failed to wait for primary: %w", err)
	}

	query, args, err := psql.Delete(leasesTable).
		Where(squirrel.Eq{"name": name, "holder": holder}).
		ToSql()
	if err != nil {
		return fmt.Errorf("failed to build query: %w", err)
	}

	_, err = primary.DBx().ExecContext(ctx, query, args...)
	if err != nil {
		return fmt.Errorf("failed to release lease: %w", err)
	}

	return nil
}
