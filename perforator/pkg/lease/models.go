package lease

import (
	"context"
	"time"
)

type Storage interface {
	// Acquire tries to acquire the lease. Returns true if acquired.
	// If the lease is already held by someone else but expired, it should be taken over.
	Acquire(ctx context.Context, name, holder string, ttl time.Duration) (bool, error)

	// Renew extends the lease if it is still held by the holder.
	// Returns true if renewed, false if the lease was lost (e.g. taken over by someone else).
	Renew(ctx context.Context, name, holder string, ttl time.Duration) (bool, error)

	// Release explicitly releases the lease if it is held by the holder.
	Release(ctx context.Context, name, holder string) error
}
