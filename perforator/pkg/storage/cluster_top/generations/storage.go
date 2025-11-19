package generations

import (
	"context"
	"fmt"
	"time"

	"github.com/Masterminds/squirrel"
	hasql "golang.yandex/hasql/sqlx"
	"google.golang.org/protobuf/types/known/timestamppb"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/pkg/foreach"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

type PostgresGenerationsStorage struct {
	logger  xlog.Logger
	cluster *hasql.Cluster
}

type clusterTopGenerationRow struct {
	ID   uint32    `db:"id"`
	From time.Time `db:"from_ts"`
	To   time.Time `db:"to_ts"`
}

func NewStorage(
	logger xlog.Logger,
	cluster *hasql.Cluster,
) *PostgresGenerationsStorage {
	return &PostgresGenerationsStorage{
		logger:  logger.WithName("ClusterTopGenerationsStorage"),
		cluster: cluster,
	}
}

func mapToProto(rows []*clusterTopGenerationRow) []*perforator.ClusterTopGeneration {
	return foreach.Map(rows, func(row *clusterTopGenerationRow) *perforator.ClusterTopGeneration {
		return &perforator.ClusterTopGeneration{
			ID:   row.ID,
			From: timestamppb.New(row.From),
			To:   timestamppb.New(row.To),
		}
	})
}

var psql = squirrel.StatementBuilder.PlaceholderFormat(squirrel.Dollar)

func (s *PostgresGenerationsStorage) ListGenerations(ctx context.Context) ([]*perforator.ClusterTopGeneration, error) {
	alive, err := s.cluster.WaitForAlive(ctx)
	if err != nil {
		return nil, fmt.Errorf("failed to wait for alive replica: %w", err)
	}
	query := psql.Select("id", "from_ts", "to_ts").
		From("cluster_top_generations").
		OrderBy("id DESC")

	sql, args, err := query.ToSql()

	if err != nil {
		return nil, fmt.Errorf("failed to build query: %w", err)
	}

	s.logger.Debug(ctx, "Listing generations in postgres", log.String("sql", sql))

	var rows []*clusterTopGenerationRow
	err = alive.DBx().SelectContext(ctx, &rows, sql, args...)
	if err != nil {
		return nil, fmt.Errorf("can't list generations: %w", err)
	}

	return mapToProto(rows), nil
}
