package signal_profile_processor

import (
	"fmt"
	"os"

	"gopkg.in/yaml.v2"

	"github.com/yandex/perforator/perforator/pkg/kafka/consumer"
	"github.com/yandex/perforator/perforator/pkg/kafka/producer"
	"github.com/yandex/perforator/perforator/symbolizer/pkg/client"
)

type Config struct {
	EventProcessorConfig
	ProxyClient   client.Config    `yaml:"proxy_client"`
	KafkaConsumer *consumer.Config `yaml:"kafka_profile_event_consumer"`
	KafkaProducer *producer.Config `yaml:"kafka_core_producer"`
}

func ParseConfig(path string) (*Config, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("can't open config file: %s", path)
	}

	var conf Config
	err = yaml.NewDecoder(file).Decode(&conf)
	if err != nil {
		return nil, fmt.Errorf("can't parse config: %s, with error: %w", path, err)
	}

	return &conf, nil
}

type EventProcessorConfig struct {
	QueueSize     int `yaml:"queue_size"`
	WorkersNumber int `yaml:"workers_number"`

	// Only these services are allowed to be processed.
	// If empty allow all.
	WhitelistServices []string `yaml:"whitelist_services"`
}

func (c *EventProcessorConfig) fillDefaults() {
	if c.QueueSize <= 0 {
		c.QueueSize = 100
	}
	if c.WorkersNumber <= 0 {
		c.WorkersNumber = 5
	}
}
