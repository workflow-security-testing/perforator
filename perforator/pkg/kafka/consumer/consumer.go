package consumer

import (
	"context"
	"encoding/json"
	"fmt"

	"github.com/segmentio/kafka-go"

	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type (
	Header  = kafka.Header
	Message = kafka.Message
)

type Consumer interface {
	Consume(ctx context.Context) (Message, error)
	Close() error
}

type KafkaConsumer struct {
	r *kafka.Reader
}

func NewKafkaConsumer(l xlog.Logger, cfg *Config) (*KafkaConsumer, error) {
	r, err := NewKafkaReader(l, cfg)
	if err != nil {
		return nil, err
	}
	return &KafkaConsumer{r: r}, nil
}

func (c *KafkaConsumer) Consume(ctx context.Context) (Message, error) {
	msg, err := c.r.ReadMessage(ctx)
	if err != nil {
		return kafka.Message{}, err
	}
	return msg, nil
}

func (c *KafkaConsumer) Close() error {
	if c.r == nil {
		return nil
	}
	return c.r.Close()
}

/////////////////////////////////////////////////////////////////////////////////////////

type JSONConsumer[T any] struct {
	base Consumer
}

func NewJSONConsumer[T any](base Consumer) *JSONConsumer[T] {
	return &JSONConsumer[T]{base: base}
}

// Consume returns JSON message.
func (j *JSONConsumer[T]) Consume(ctx context.Context) (value *T, err error) {
	msg, err := j.base.Consume(ctx)
	if err != nil {
		return nil, err
	}
	var out T
	if err := json.Unmarshal(msg.Value, &out); err != nil {
		return nil, fmt.Errorf("unmarshal json from kafka: %w", err)
	}
	return &out, nil
}

func (j *JSONConsumer[T]) Close() error {
	return j.base.Close()
}
