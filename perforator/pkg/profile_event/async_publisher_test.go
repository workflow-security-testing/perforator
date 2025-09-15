package profile_event

import (
	"context"
	"errors"
	"math/rand"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"

	coreLog "github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/log/zap"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/kafka/producer"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

func newEvent() *SignalProfileEvent {
	return &SignalProfileEvent{
		ProfileID:   "id",
		Service:     "svc",
		Cluster:     "cluster",
		NodeID:      "node-1",
		PodID:       "pod-1",
		Timestamp:   time.Now().UTC().Round(time.Millisecond),
		BuildIDs:    []string{"b1", "b2"},
		MainEvent:   "signal.count",
		SignalTypes: []string{"SIGSEGV", "SIGKILL"},
	}
}

type blockingProducer struct {
	closed  atomic.Bool
	unblock chan struct{}
}

func (b *blockingProducer) Produce(ctx context.Context, key, val []byte, headers ...producer.Header) error {
	select {
	case <-ctx.Done():
		return ctx.Err()
	case <-b.unblock:
		return nil
	}
}

func (b *blockingProducer) Close() error {
	if b.closed.Swap(true) {
		return nil
	}
	close(b.unblock)
	return nil
}

func TestDropWhenQueueFull(t *testing.T) {
	bp := &blockingProducer{unblock: make(chan struct{}, 1)}
	logger := xlog.ForTest(t)
	reg := xmetrics.NewRegistry()

	p := NewAsyncSignalProfileEventPublisher(bp, logger, reg, Config{QueueSize: 1, WorkersNumber: 1})

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	go func() { _ = p.Run(ctx) }()

	ev := newEvent()
	// Should get into the queue.
	p.TryEnqueueForPublish(ctx, ev)
	time.Sleep(100 * time.Millisecond)
	assert.Len(t, p.messageChan, 0, "first send")

	// Should get into the queue: queue is empty, but worker is blocked on writer,
	p.TryEnqueueForPublish(ctx, ev)
	time.Sleep(100 * time.Millisecond)
	assert.Len(t, p.messageChan, 1, "second send")

	// Third enqueue should drop: queue full
	p.TryEnqueueForPublish(ctx, ev)
	time.Sleep(100 * time.Millisecond)
	assert.Len(t, p.messageChan, 1, "third send")

	_ = bp.Close()
}

type countingProducer struct {
	closed atomic.Bool

	callCount   atomic.Int32
	delay       time.Duration
	maxInFlight atomic.Int32
	inFlight    atomic.Int32
	failFirstN  int32
}

func (c *countingProducer) Produce(ctx context.Context, key, val []byte, headers ...producer.Header) error {
	cur := c.inFlight.Add(1)
	for {
		max := c.maxInFlight.Load()
		if cur > max && c.maxInFlight.CompareAndSwap(max, cur) {
			break
		}
		if cur <= max {
			break
		}
	}
	defer c.inFlight.Add(-1)

	n := c.callCount.Add(1)

	if c.delay > 0 {
		select {
		case <-ctx.Done():
			return ctx.Err()
		case <-time.After(c.delay):
		}
	}

	if c.failFirstN > 0 && n <= c.failFirstN {
		return errors.New("error")
	}
	return nil
}

func (c *countingProducer) Close() error {
	c.closed.Store(true)
	return nil
}

func TestParallelWorkers(t *testing.T) {
	logger, _ := xlog.TryNew(zap.NewDeployLogger(coreLog.DebugLevel))
	reg := xmetrics.NewRegistry()

	const workernumber = 4
	pr := &countingProducer{delay: 50 * time.Millisecond}
	pub := NewAsyncSignalProfileEventPublisher(pr, logger, reg, Config{
		QueueSize:     100,
		WorkersNumber: workernumber,
	})

	ctx, cancel := context.WithCancel(context.Background())
	go func() { _ = pub.Run(ctx) }()

	// Enqueue more events than workers.
	ev := newEvent()
	for range 10 {
		pub.TryEnqueueForPublish(ctx, ev)
	}

	time.Sleep(150 * time.Millisecond)
	cancel()

	got := pr.callCount.Load()
	assert.Greater(t, got, int32(0), "expected some publishes")

	maxInflight := pr.maxInFlight.Load()
	assert.Greater(t, maxInflight, int32(1), "expected parallelism")
	assert.LessOrEqual(t, maxInflight, int32(workernumber), "exceeded max amount of workers")
}

func TestCancelStopsAndClosesProducer(t *testing.T) {
	logger, _ := xlog.TryNew(zap.NewDeployLogger(coreLog.DebugLevel))
	reg := xmetrics.NewRegistry()
	delay := 10 * time.Millisecond
	pr := &countingProducer{delay: delay}
	pub := NewAsyncSignalProfileEventPublisher(pr, logger, reg, Config{
		QueueSize:     10,
		WorkersNumber: 2,
	})

	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { done <- pub.Run(ctx) }()

	ev := newEvent()
	n := 10
	for range n {
		pub.TryEnqueueForPublish(ctx, ev)
	}

	time.Sleep(20 * time.Millisecond)
	cancel()

	err := <-done

	assert.Error(t, err, "Run should return an error on stop")
	assert.True(t, pr.closed.Load(), "underlying producer Close was not called")

}

func TestTryEnqueueAfterCancel(t *testing.T) {
	logger, _ := xlog.TryNew(zap.NewDeployLogger(coreLog.DebugLevel))
	reg := xmetrics.NewRegistry()

	pr := &countingProducer{}
	pub := NewAsyncSignalProfileEventPublisher(pr, logger, reg, Config{
		QueueSize:     1,
		WorkersNumber: 1,
	})

	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { done <- pub.Run(ctx) }()

	ev := newEvent()
	pub.TryEnqueueForPublish(ctx, ev)
	time.Sleep(10 * time.Millisecond)

	cancel()
	err := <-done
	assert.Error(t, err)

	pub.TryEnqueueForPublish(ctx, ev)
	pub.TryEnqueueForPublish(ctx, ev)
	assert.Equal(t, pr.callCount.Load(), int32(1), "expected 1 publish before Close")
}

func TestErrorsDoNotBlockSubsequentPublishes(t *testing.T) {
	logger, _ := xlog.TryNew(zap.NewDeployLogger(coreLog.DebugLevel))
	reg := xmetrics.NewRegistry()

	// Fail first 3.
	pr := &countingProducer{failFirstN: 3}
	pub := NewAsyncSignalProfileEventPublisher(pr, logger, reg, Config{
		QueueSize:     20,
		WorkersNumber: 3,
	})

	ctx, cancel := context.WithCancel(context.Background())
	go func() { _ = pub.Run(ctx) }()

	ev := newEvent()
	n := 8
	for i := 0; i < n; i++ {
		pub.TryEnqueueForPublish(ctx, ev)
	}

	time.Sleep(100 * time.Millisecond)
	cancel()

	assert.Equal(t, pr.callCount.Load(), int32(n), "expected all calls even with failures")
}

func TestManyConcurrentTryEnqueue(t *testing.T) {
	logger, _ := xlog.TryNew(zap.NewDeployLogger(coreLog.InfoLevel))
	reg := xmetrics.NewRegistry()

	pr := &countingProducer{delay: 2 * time.Millisecond}
	pub := NewAsyncSignalProfileEventPublisher(pr, logger, reg, Config{
		QueueSize:     700,
		WorkersNumber: 4,
	})

	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { done <- pub.Run(ctx) }()

	start := make(chan struct{})
	var wg sync.WaitGroup
	n := 4500
	wg.Add(n)

	ev := newEvent()
	for i := 0; i < n; i++ {
		go func() {
			defer wg.Done()
			<-start
			time.Sleep(time.Duration(rand.Intn(500)) * time.Microsecond)
			pub.TryEnqueueForPublish(ctx, ev)
		}()
	}

	// Start all.
	close(start)
	time.Sleep(1 * time.Microsecond)
	// Some TryEnqueue calls may still be in progress.
	cancel()

	wg.Wait()

	err := <-done
	assert.Error(t, err, "Run should return context error")
	assert.True(t, pr.closed.Load(), "underlying producer should be closed")

	got := pr.callCount.Load()
	assert.Greater(t, got, int32(0), "expected at least some publishes under load")
	assert.LessOrEqual(t, got, int32(n), "cannot publish more than attempts")
}
