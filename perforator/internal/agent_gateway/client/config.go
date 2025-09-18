package client

import (
	"time"

	"github.com/yandex/perforator/perforator/internal/agent_gateway/client/storage"
	"github.com/yandex/perforator/perforator/pkg/certifi"
	"github.com/yandex/perforator/perforator/pkg/endpointsetresolver"
	"github.com/yandex/perforator/perforator/pkg/grpcutil/interceptors/rate_limit"
)

type Config struct {
	TvmConfig   *TvmConfig                            `yaml:"tvm"`
	TLS         certifi.ClientTLSConfig               `yaml:"tls"`
	GRPCConfig  GRPCConfig                            `yaml:"grpc,omitempty"`
	EndpointSet endpointsetresolver.EndpointSetConfig `yaml:"endpoint_set,omitempty"`
	Host        string                                `yaml:"host,omitempty"`
	Port        uint32                                `yaml:"port,omitempty"`
	Retry       RetryConfig                           `yaml:"retry,omitempty"`
	RateLimit   rate_limit.Config                     `yaml:"rate_limit,omitempty"`

	StorageClient storage.Config `yaml:",inline"`

	CertificateNameDeprecated string `yaml:"name,omitempty"`
	CACertPathDeprecated      string `yaml:"ca_cert_path,omitempty"`
}

func (c *Config) FillDefault() {
	c.Retry.fillDefault()
	c.GRPCConfig.fillDefault()

	// TLS backward compatibility.
	// Previously, agent communicated with storage via only TLS, so you need to enable tls if these values ​​are present.
	if c.CertificateNameDeprecated != "" {
		c.TLS.Enabled = true
		c.TLS.ServerNameOverride = c.CertificateNameDeprecated
	}

	if c.CACertPathDeprecated != "" {
		c.TLS.Enabled = true
		c.TLS.CAFile = c.CACertPathDeprecated
	}

	if len(c.RateLimit.Methods) == 0 {
		c.RateLimit.Methods = []rate_limit.RateLimitedMethod{
			{
				Path:       "/NPerforator.NProto.NCustomProfilingOperation.CustomProfilingOperationService/PollOperations",
				AverageRPS: 2,
				MaxRPS:     5,
			},
		}
	}
}

type TvmConfig struct {
	SecretVar        string `yaml:"tvm_secret_var"`
	ServiceFromTvmID uint32 `yaml:"from_service_id"`
	ServiceToTvmID   uint32 `yaml:"to_service_id"`
	CacheDir         string `yaml:"cache_dir"`
}

type GRPCConfig struct {
	MaxSendMessageSize uint32 `yaml:"max_send_message_size"`
}

func (c *GRPCConfig) fillDefault() {
	if c.MaxSendMessageSize == 0 {
		c.MaxSendMessageSize = 1024 * 1024 * 1024 // 1 GB
	}
}

// RetryConfig defines settings for gRPC retry policy for client
type RetryConfig struct {
	MaxAttempts          int           `yaml:"max_attempts"`
	InitialBackoff       time.Duration `yaml:"initial_backoff"`
	MaxBackoff           time.Duration `yaml:"max_backoff"`
	BackoffMultiplier    float64       `yaml:"backoff_multiplier"`
	RetryableStatusCodes []string      `yaml:"retryable_status_codes"`
}

func (r *RetryConfig) fillDefault() {
	if r.MaxAttempts == 0 {
		r.MaxAttempts = 5
	}
	if r.InitialBackoff == time.Duration(0) {
		r.InitialBackoff = 200 * time.Millisecond
	}
	if r.MaxBackoff == time.Duration(0) {
		r.MaxBackoff = 5 * time.Second
	}
	if r.BackoffMultiplier == 0 {
		r.BackoffMultiplier = 2
	}
	if len(r.RetryableStatusCodes) == 0 {
		r.RetryableStatusCodes = []string{"CANCELLED", "UNKNOWN", "RESOURCE_EXHAUSTED", "INTERNAL", "UNAVAILABLE"}
	}
}
