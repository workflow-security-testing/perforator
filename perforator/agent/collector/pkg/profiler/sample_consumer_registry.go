package profiler

import (
	"errors"
	"fmt"
	"sync"
)

var (
	errAlreadyRegistered = errors.New("sample consumer is already registered")
)

type SampleConsumerRegistry interface {
	Register(name string, consumer SampleConsumer) error
	Unregister(name string)
	Get(name string) SampleConsumer
	Consumers() []SampleConsumer
}

type sampleConsumerRegistry struct {
	sync.RWMutex
	consumers map[string]SampleConsumer
}

func newSampleConsumerRegistry() *sampleConsumerRegistry {
	return &sampleConsumerRegistry{
		consumers: make(map[string]SampleConsumer),
	}
}

func (r *sampleConsumerRegistry) Register(name string, consumer SampleConsumer) error {
	r.Lock()
	defer r.Unlock()

	_, ok := r.consumers[name]
	if ok {
		return fmt.Errorf("consumer %s: %w", name, errAlreadyRegistered)
	}

	r.consumers[name] = consumer
	return nil
}

func (r *sampleConsumerRegistry) Unregister(name string) {
	r.Lock()
	defer r.Unlock()

	delete(r.consumers, name)
}

func (r *sampleConsumerRegistry) Get(name string) SampleConsumer {
	r.RLock()
	defer r.RUnlock()

	return r.consumers[name]
}

func (r *sampleConsumerRegistry) Consumers() (res []SampleConsumer) {
	r.RLock()
	defer r.RUnlock()

	// for now just return all consumers
	for _, consumer := range r.consumers {
		res = append(res, consumer)
	}
	return res
}
