package binary

import (
	"context"

	"github.com/yandex/perforator/perforator/agent/preprocessing/proto/parse"
)

type Listener interface {
	// OnBinaryDiscovery is called when a new (or old, but fully evicted) binary is discovered.
	// The callback is not allowed to modify its arguments.
	OnBinaryDiscovery(ctx context.Context, binaryID uint64, buildID string, analysis *parse.BinaryAnalysis)
}
