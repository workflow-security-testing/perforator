package servicediscovery

import (
	"time"

	"github.com/yandex/perforator/perforator/pkg/endpointsetresolver"
)

type DiscovererType string

type Config struct {
	EndpointSetConfig []endpointsetresolver.EndpointSetConfig `yaml:"endpoint_set"`
	Endpoints         []string                                `yaml:"endpoints"`
	RequestInterval   time.Duration                           `yaml:"request_interval"`
	Discoverer        DiscovererType                          `yaml:"discoverer"`
}

func (c *Config) FillDefault() {
	if c.RequestInterval == 0 {
		c.RequestInterval = 5 * time.Second
	}
}
