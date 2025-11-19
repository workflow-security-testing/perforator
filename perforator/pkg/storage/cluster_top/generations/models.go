package generations

import (
	"context"

	"github.com/yandex/perforator/perforator/proto/perforator"
)

type GenerationsStorage interface {
	ListGenerations(ctx context.Context) ([]*perforator.ClusterTopGeneration, error)
}
