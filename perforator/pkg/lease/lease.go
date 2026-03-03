package lease

import (
	"context"
	"crypto/rand"
	"encoding/base64"
	"errors"
	"fmt"
	"os"
	"sync/atomic"
	"time"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/metrics"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

// BuildPerProcessHolderID generates a unique identifier for a lease holder based on the hostname
// and random bytes encoded in base64.
func BuildPerProcessHolderID() (string, error) {
	hostname, err := os.Hostname()
	if err != nil {
		return "", fmt.Errorf("failed to get hostname: %w", err)
	}
	b := make([]byte, 6)
	if _, err := rand.Read(b); err != nil {
		return "", fmt.Errorf("failed to read random bytes: %w", err)
	}
	return fmt.Sprintf("%s-%s", hostname, base64.RawURLEncoding.EncodeToString(b)), nil
}

type leaseOptions struct {
	ttl                  time.Duration
	renewInterval        time.Duration
	maxAcquireRetries    uint32
	acquireRetryInterval time.Duration
	registry             metrics.Registry
}

type LeaseOption func(*leaseOptions)

func WithTTL(ttl time.Duration) LeaseOption {
	return func(o *leaseOptions) {
		o.ttl = ttl
	}
}

func WithRenewInterval(interval time.Duration) LeaseOption {
	return func(o *leaseOptions) {
		o.renewInterval = interval
	}
}

func WithMaxAcquireRetries(retries uint32) LeaseOption {
	return func(o *leaseOptions) {
		o.maxAcquireRetries = retries
	}
}

func WithMetrics(registry metrics.Registry) LeaseOption {
	return func(o *leaseOptions) {
		o.registry = registry
	}
}

func defaultLeaseOptions() leaseOptions {
	return leaseOptions{
		ttl:               30 * time.Second,
		maxAcquireRetries: 5,
	}
}

var (
	ErrLeaseLost = errors.New("lease was lost")
)

// leaseHolder manages a single distributed lease.
type leaseHolder struct {
	storage  Storage
	logger   xlog.Logger
	name     string
	holderID string
	options  leaseOptions

	leaseAcquireTime atomic.Int64 // UnixNano

	leaseCtx  context.Context
	cancel    context.CancelCauseFunc
	done      chan struct{}
	leaseHeld metrics.Gauge
}

// newLeaseHolder creates a new leaseHolder for a specific lease and holder.
func newLeaseHolder(
	logger xlog.Logger,
	storage Storage,
	leaseName string,
	holderID string,
	opts ...LeaseOption,
) *leaseHolder {
	options := defaultLeaseOptions()
	for _, opt := range opts {
		opt(&options)
	}
	if options.renewInterval == 0 {
		options.renewInterval = options.ttl / 3
	}
	if options.acquireRetryInterval == 0 {
		options.acquireRetryInterval = options.ttl / 3
	}

	h := &leaseHolder{
		logger:   logger.WithName("LeaseHolder").With(log.String("lease_name", leaseName), log.String("holder_id", holderID)),
		storage:  storage,
		name:     leaseName,
		holderID: holderID,
		options:  options,
	}

	if options.registry != nil {
		h.leaseHeld = options.registry.WithTags(map[string]string{
			"name": leaseName,
		}).Gauge("lease.held")
	}

	return h
}

// hold attempts to acquire the lease, retrying if it is already held or if transient errors occur.
// If successful, it starts a background goroutine to keep the lease alive.
// It blocks until the lease is acquired, the maximum number of retries for storage errors is exceeded,
// or the provided context is canceled.
// The lease lifetime is tied to the context passed to this method.
// If the context is canceled, the lease will be released.
func (h *leaseHolder) hold(ctx context.Context) error {
	retryErrors := []error{}
	for {
		acquireTime := time.Now()
		acquired, err := h.storage.Acquire(ctx, h.name, h.holderID, h.options.ttl)
		if err == nil && acquired {
			h.leaseAcquireTime.Store(acquireTime.UnixNano())
			h.leaseCtx, h.cancel = context.WithCancelCause(ctx)
			h.done = make(chan struct{})
			go h.runProlongation()
			if h.leaseHeld != nil {
				h.leaseHeld.Set(1)
			}
			return nil
		}

		if err != nil {
			h.logger.Warn(ctx, "Failed to acquire lease", log.Error(err))
			retryErrors = append(retryErrors, err)
		} else if !acquired {
			h.logger.Debug(ctx, "Lease is already held")
			retryErrors = retryErrors[:0]
		}

		if len(retryErrors) >= int(h.options.maxAcquireRetries) {
			return fmt.Errorf("failed to acquire lease after %d retries: %w", len(retryErrors), errors.Join(retryErrors...))
		}

		select {
		case <-ctx.Done():
			return ctx.Err()
		case <-time.After(h.options.acquireRetryInterval):
			continue
		}
	}
}

// context returns a context that is canceled if the lease is lost or released.
// Returns nil if the lease has not been acquired.
func (h *leaseHolder) context() context.Context {
	return h.leaseCtx
}

// close stops the renewal process and releases the lease.
// It blocks until the background renewal goroutine has finished.
func (h *leaseHolder) close() error {
	if h.cancel == nil {
		return nil
	}

	if h.leaseHeld != nil {
		h.leaseHeld.Set(0)
	}

	h.cancel(nil)
	if h.done != nil {
		<-h.done
	}

	deadline := time.Unix(0, h.leaseAcquireTime.Load()).Add(h.options.ttl)

	releaseCtx, cancelReleaseCtx := context.WithDeadline(context.WithoutCancel(h.leaseCtx), deadline)
	defer cancelReleaseCtx()

	releaseErr := h.storage.Release(releaseCtx, h.name, h.holderID)
	if releaseErr != nil {
		h.logger.Warn(releaseCtx, "Failed to release lease", log.Error(releaseErr))
	}

	return nil
}

func (h *leaseHolder) runProlongation() {
	defer close(h.done)
	ticker := time.NewTicker(h.options.renewInterval)
	defer ticker.Stop()

	for {
		acquireTimeNano := h.leaseAcquireTime.Load()
		leaseExpiresAt := time.Unix(0, acquireTimeNano).Add(h.options.ttl)

		renewErrors := []error{}

		select {
		case <-h.leaseCtx.Done():
			return
		case <-ticker.C:
			renewed, err := h.storage.Renew(h.leaseCtx, h.name, h.holderID, h.options.ttl)
			if err != nil {
				h.logger.Warn(h.leaseCtx, "Failed to renew lease", log.Error(err))
				renewErrors = append(renewErrors, err)

				if time.Now().After(leaseExpiresAt) {
					if h.leaseHeld != nil {
						h.leaseHeld.Set(0)
					}
					h.cancel(fmt.Errorf("lease expired due to renewal failures: %w", errors.Join(renewErrors...)))
					return
				}
				continue
			} else {
				clear(renewErrors)
			}

			if !renewed {
				if h.leaseHeld != nil {
					h.leaseHeld.Set(0)
				}
				h.cancel(ErrLeaseLost)
				return
			}

			h.logger.Debug(h.leaseCtx, "Lease renewed")

			h.leaseAcquireTime.Store(time.Now().UnixNano())
		}
	}
}

// LockAndRun tries to acquire a lease with the given name.
// If successful, it executes the action function.
// The action function receives a context that is canceled if the lease is lost or released.
// If the lease is already held, it returns an error.
func LockAndRun(
	ctx context.Context,
	logger xlog.Logger,
	storage Storage,
	leaseName string,
	holderID string,
	action func(ctx context.Context),
	opts ...LeaseOption,
) error {
	holder := newLeaseHolder(logger, storage, leaseName, holderID, opts...)

	if err := holder.hold(ctx); err != nil {
		return err
	}

	defer func() {
		if closeErr := holder.close(); closeErr != nil {
			logger.Warn(ctx, "Failed to close lease holder", log.Error(closeErr))
		}
	}()

	if ctx.Err() != nil {
		return ctx.Err()
	}

	action(holder.context())

	return nil
}
