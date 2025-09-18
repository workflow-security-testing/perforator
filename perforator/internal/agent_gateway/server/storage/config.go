package storage

import (
	kafka "github.com/yandex/perforator/perforator/pkg/kafka/producer"
	"github.com/yandex/perforator/perforator/pkg/profile_event"
	"github.com/yandex/perforator/perforator/pkg/storage/microscope/filter"
)

type ProfileSignalEventsConfig struct {
	profile_event.Config
	AllowedSignals []string      `yaml:"allowed_signals"` // e.g, SIGSEGV, SIGQUIT
	Kafka          *kafka.Config `yaml:"kafka"`
}

type ServiceConfig struct {
	MicroscopePullerConfig *filter.Config             `yaml:"microscope_puller"`
	ProfileSignalEvents    *ProfileSignalEventsConfig `yaml:"profile_signal_events"`
}
