package producer

import (
	"context"
	"errors"
	"fmt"
	"os"
	"time"

	"github.com/segmentio/kafka-go"
	"github.com/segmentio/kafka-go/sasl/plain"
	"github.com/segmentio/kafka-go/sasl/scram"

	"github.com/yandex/perforator/perforator/pkg/certifi"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type Config struct {
	Topic         string                  `yaml:"topic"`
	Brokers       []string                `yaml:"brokers"`
	User          string                  `yaml:"user"`
	PasswordEnv   string                  `yaml:"password_env"`
	SASLMechanism string                  `yaml:"sasl_mechanism"`
	WriteTimeout  time.Duration           `yaml:"message_timeout"`
	Retries       int                     `yaml:"retries"`
	TLS           certifi.ClientTLSConfig `yaml:"tls"`
}

func (cfg *Config) FillDefault() {
	if cfg.Retries <= 0 {
		cfg.Retries = 3
	}
	if cfg.WriteTimeout <= 0 {
		cfg.WriteTimeout = 10 * time.Second
	}
}

// NewKafkaWriter creates a new Kafka publisher
func NewKafkaWriter(ctx context.Context, l xlog.Logger, cfg *Config) (*kafka.Writer, error) {
	cfg.FillDefault()
	l = l.WithName("kafka")

	if cfg.Topic == "" {
		return nil, errors.New("kafka topic is required")
	}
	if len(cfg.Brokers) == 0 {
		return nil, errors.New("kafka brokers are required")
	}

	kafkaTransport := &kafka.Transport{}
	if cfg.User != "" {
		password := os.Getenv(cfg.PasswordEnv)
		if password == "" {
			return nil, fmt.Errorf("kafka password environment variable %s is not set", cfg.PasswordEnv)
		}
		switch cfg.SASLMechanism {
		case scram.SHA512.Name():
			mechanism, err := scram.Mechanism(scram.SHA512, cfg.User, password)
			if err != nil {
				return nil, fmt.Errorf("unable to init scram: %w", err)
			}
			kafkaTransport.SASL = mechanism
		case plain.Mechanism{}.Name(), "":
			kafkaTransport.SASL = plain.Mechanism{
				Username: cfg.User,
				Password: password,
			}
		default:
			return nil, errors.New("unknown SASL mechanism")
		}
	}

	if cfg.TLS.Enabled {
		tlsConf, err := cfg.TLS.BuildTLSConfig()
		if err != nil {
			return nil, fmt.Errorf("failed to configure TLS: %w", err)
		}
		kafkaTransport.TLS = tlsConf
	}

	writer := &kafka.Writer{
		Addr:         kafka.TCP(cfg.Brokers...),
		Topic:        cfg.Topic,
		Balancer:     &kafka.Hash{},
		Async:        false,
		RequiredAcks: kafka.RequireOne,
		Logger:       kafka.LoggerFunc(l.Fmt().Infof),
		ErrorLogger:  kafka.LoggerFunc(l.Fmt().Errorf),
		WriteTimeout: cfg.WriteTimeout,
		BatchTimeout: cfg.WriteTimeout,
		MaxAttempts:  cfg.Retries,
		Transport:    kafkaTransport,
	}

	return writer, nil
}
