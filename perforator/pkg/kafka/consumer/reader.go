package consumer

import (
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/segmentio/kafka-go"
	"github.com/segmentio/kafka-go/sasl/scram"

	"github.com/yandex/perforator/perforator/pkg/certifi"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type Config struct {
	Topic         string                  `yaml:"topic"`
	Brokers       []string                `yaml:"brokers"`
	GroupID       string                  `yaml:"group_id"`
	User          string                  `yaml:"user"`
	PasswordEnv   string                  `yaml:"password_env"`
	MinBytes      int                     `yaml:"min_bytes"` // preferred minimum fetch size
	MaxBytes      int                     `yaml:"max_bytes"` // maximum fetch size
	MaxWait       time.Duration           `yaml:"max_wait"`
	QueueCapacity int                     `yaml:"queue_capacity"`
	TLS           certifi.ClientTLSConfig `yaml:"tls"`
}

func (c *Config) FillDefault() {
	if c.MinBytes <= 0 {
		c.MinBytes = 1
	}
	if c.MaxBytes <= 0 {
		c.MaxBytes = 10 * 1024 * 1024
	}
	if c.MaxWait <= 0 {
		c.MaxWait = 10 * time.Second
	}
	if c.QueueCapacity <= 0 {
		c.QueueCapacity = 1024
	}
}

// NewKafkaReader builds a kafka-go Reader.
func NewKafkaReader(l xlog.Logger, cfg *Config) (*kafka.Reader, error) {
	cfg.FillDefault()
	l = l.WithName("kafka")

	if cfg.Topic == "" {
		return nil, errors.New("kafka topic is required")
	}
	if len(cfg.Brokers) == 0 {
		return nil, errors.New("kafka brokers are required")
	}
	if cfg.GroupID == "" {
		return nil, errors.New("kafka group_id is required")
	}

	var dialer = &kafka.Dialer{}
	if cfg.User != "" {
		password := os.Getenv(cfg.PasswordEnv)
		if password == "" {
			return nil, fmt.Errorf("kafka password environment variable %s is not set", cfg.PasswordEnv)
		}
		mech, err := scram.Mechanism(scram.SHA512, cfg.User, password)
		if err != nil {
			return nil, fmt.Errorf("unable to init scram: %w", err)
		}
		dialer.SASLMechanism = mech
	}

	if cfg.TLS.Enabled {
		tlsConf, err := cfg.TLS.BuildTLSConfig()
		if err != nil {
			return nil, fmt.Errorf("failed to configure TLS: %w", err)
		}
		dialer.TLS = tlsConf
	}

	reader := kafka.NewReader(kafka.ReaderConfig{
		Brokers:       cfg.Brokers,
		Topic:         cfg.Topic,
		GroupID:       cfg.GroupID,
		MinBytes:      cfg.MinBytes,
		MaxBytes:      cfg.MaxBytes,
		MaxWait:       cfg.MaxWait,
		QueueCapacity: cfg.QueueCapacity,
		Dialer:        dialer,
		Logger:        kafka.LoggerFunc(l.Fmt().Infof),
		ErrorLogger:   kafka.LoggerFunc(l.Fmt().Errorf),
	})

	return reader, nil
}
