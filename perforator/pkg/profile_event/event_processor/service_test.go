package event_processor

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"

	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/kafka/consumer"
	"github.com/yandex/perforator/perforator/pkg/kafka/producer"
	"github.com/yandex/perforator/perforator/pkg/profile_event"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

func newSignalEvent(svc string) *profile_event.SignalProfileEvent {
	return &profile_event.SignalProfileEvent{
		ProfileID: "p-" + svc,
		Service:   svc,
		Cluster:   "k8s",
		NodeID:    "node-1",
		PodID:     "pod-1",
		Timestamp: time.Now().UTC().Round(time.Millisecond),
	}
}

func TestProcessesAndPublishes(t *testing.T) {
	logger := xlog.ForTest(t)
	reg := xmetrics.NewRegistry()

	fc := &testConsumer{}
	fp := &testProducer{}
	proc := &testProcessor{}

	cfg := EventProcessorConfig{
		QueueSize:         16,
		WorkersNumber:     2,
		WhitelistServices: []string{"svc-a", "svc-b"},
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	svc, err := newEventProcessor(fp, fc, proc, logger, cfg, reg)
	assert.NoError(t, err)

	evA := newSignalEvent("svc-a")
	evB := newSignalEvent("svc-b")
	evX := newSignalEvent("svc-x") // not in whitelist

	fc.enqueue(consumer.Message{Key: []byte(evA.Service), Value: encodeSignalEvent(evA)})
	fc.enqueue(consumer.Message{Key: []byte(evB.Service), Value: encodeSignalEvent(evB)})
	fc.enqueue(consumer.Message{Key: []byte(evX.Service), Value: encodeSignalEvent(evX)})

	done := make(chan struct{})
	go func() {
		_ = svc.Run(ctx)
		close(done)
	}()

	time.Sleep(150 * time.Millisecond)
	cancel()
	<-done

	// verify svc-x filtered
	out := fp.dump()
	assert.Len(t, out, 2, "expected two published messages")

	for _, m := range out {
		var ce profile_event.CoreEvent
		err := json.Unmarshal(m.Value, &ce)
		assert.NoError(t, err)
		assert.Contains(t, []string{"svc-a", "svc-b"}, ce.Service)
	}
}

func TestWorkersParallel(t *testing.T) {
	logger := xlog.ForTest(t)
	reg := xmetrics.NewRegistry()

	fc := &testConsumer{}
	fp := &testProducer{}
	proc := &testProcessor{Delay: 40 * time.Millisecond}

	cfg := EventProcessorConfig{
		QueueSize:         64,
		WorkersNumber:     4,
		WhitelistServices: nil, // allow all
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	svc, err := newEventProcessor(fp, fc, proc, logger, cfg, reg)
	assert.NoError(t, err)

	for i := 0; i < 8; i++ {
		ev := newSignalEvent(fmt.Sprintf("svc-%d", i))
		fc.enqueue(consumer.Message{Key: []byte(ev.Service), Value: encodeSignalEvent(ev)})
	}

	start := time.Now()
	done := make(chan struct{})
	go func() { _ = svc.Run(ctx); close(done) }()

	time.Sleep(150 * time.Millisecond)
	cancel()
	<-done
	elapsed := time.Since(start)

	assert.Less(t, elapsed, 300*time.Millisecond, "processing did not look parallel")
	assert.GreaterOrEqual(t, len(fp.dump()), 1, "expected some published messages")
}

func TestRunProcessorErrorsDontBlock(t *testing.T) {
	logger := xlog.ForTest(t)
	reg := xmetrics.NewRegistry()

	fc := &testConsumer{}
	fp := &testProducer{}
	proc := &testProcessor{FailFirstN: 3} // first 3 calls fail

	cfg := EventProcessorConfig{
		QueueSize:         20,
		WorkersNumber:     2,
		WhitelistServices: nil,
	}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	svc, err := newEventProcessor(fp, fc, proc, logger, cfg, reg)
	assert.NoError(t, err)

	for i := 0; i < 6; i++ {
		ev := newSignalEvent("svc")
		fc.enqueue(consumer.Message{Key: []byte(ev.Service), Value: encodeSignalEvent(ev)})
	}

	done := make(chan struct{})
	go func() { _ = svc.Run(ctx); close(done) }()

	time.Sleep(150 * time.Millisecond)
	cancel()
	<-done

	assert.GreaterOrEqual(t, len(fp.dump()), 3, "successful messages should still get published")
}

type testProcessor struct {
	Delay time.Duration

	// Make the processor return an error for the first N calls.
	FailFirstN int32
	calls      atomic.Int32
}

func (p *testProcessor) Process(ctx context.Context, in *profile_event.SignalProfileEvent) (*profile_event.CoreMessage, error) {
	n := p.calls.Add(1)
	if p.FailFirstN > 0 && n <= p.FailFirstN {
		return nil, errors.New("processor injected error")
	}
	if p.Delay > 0 {
		select {
		case <-ctx.Done():
			return nil, ctx.Err()
		case <-time.After(p.Delay):
		}
	}
	out := &profile_event.CoreEvent{
		Service:    in.Service,
		Type:       "go",
		Cluster:    in.Cluster,
		PodID:      in.PodID,
		NodeID:     in.NodeID,
		Signal:     "SIGSEGV",
		Message:    "panic",
		Timestamp:  in.Timestamp.Unix(),
		Attributes: map[string]string{"test": "1"},
		Traceback:  "trace",
	}
	return &profile_event.CoreMessage{
		PartitionKey: in.Service,
		Event:        out,
	}, nil
}

type testProducer struct {
	mu       sync.Mutex
	closed   atomic.Bool
	messages []producedMsg
}

type producedMsg struct {
	Key     []byte
	Value   []byte // JSON encoded CoreEvent
	Headers []producer.Header
}

func (p *testProducer) Produce(_ context.Context, key, val []byte, headers ...producer.Header) error {
	if p.closed.Load() {
		return errors.New("producer closed")
	}
	p.mu.Lock()
	p.messages = append(p.messages, producedMsg{Key: key, Value: val, Headers: headers})
	p.mu.Unlock()
	return nil
}

func (p *testProducer) Close() error {
	p.closed.Store(true)
	return nil
}

func (p *testProducer) dump() []producedMsg {
	p.mu.Lock()
	defer p.mu.Unlock()
	out := make([]producedMsg, len(p.messages))
	copy(out, p.messages)
	return out
}

type testConsumer struct {
	mu      sync.Mutex
	closed  atomic.Bool
	queue   []consumer.Message
	waiters []chan struct{}
}

type rawMsg struct {
	Key     []byte
	Value   []byte // JSON encoded SignalProfileEvent
	Headers []consumer.Header
}

func (c *testConsumer) enqueue(m consumer.Message) {
	c.mu.Lock()
	c.queue = append(c.queue, m)
	// wake one waiter
	if len(c.waiters) > 0 {
		w := c.waiters[0]
		copy(c.waiters[0:], c.waiters[1:])
		c.waiters = c.waiters[:len(c.waiters)-1]
		close(w)
	}
	c.mu.Unlock()
}

func (c *testConsumer) Consume(ctx context.Context) (m consumer.Message, err error) {
	for {
		c.mu.Lock()
		if c.closed.Load() {
			c.mu.Unlock()
			return consumer.Message{}, errors.New("consumer closed")
		}
		if len(c.queue) > 0 {
			m := c.queue[0]
			copy(c.queue[0:], c.queue[1:])
			c.queue = c.queue[:len(c.queue)-1]
			c.mu.Unlock()
			return m, nil
		}
		// wait for enqueue or ctx cancel
		ch := make(chan struct{})
		c.waiters = append(c.waiters, ch)
		c.mu.Unlock()

		select {
		case <-ctx.Done():
			return consumer.Message{}, ctx.Err()
		case <-ch:
		}
	}
}

func (c *testConsumer) Close() error {
	c.closed.Store(true)
	c.mu.Lock()
	for _, w := range c.waiters {
		close(w)
	}
	c.waiters = nil
	c.mu.Unlock()
	return nil
}

func encodeSignalEvent(ev *profile_event.SignalProfileEvent) []byte {
	b, _ := json.Marshal(ev)
	return b
}
