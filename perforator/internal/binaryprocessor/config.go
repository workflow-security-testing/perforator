package binaryprocessor

import (
	"os"

	"gopkg.in/yaml.v3"

	proxy "github.com/yandex/perforator/perforator/internal/symbolizer/proxy/server"
	"github.com/yandex/perforator/perforator/pkg/storage/bundle"
)

type Config struct {
	StorageConfig       bundle.Config              `yaml:"storage"`
	BinaryProvider      proxy.BinaryProviderConfig `yaml:"binary_provider"`
	SymbolizationConfig proxy.SymbolizationConfig  `yaml:"symbolization"`
}

func ParseConfig(path string) (conf *Config, err error) {
	var file *os.File
	file, err = os.Open(path)
	if err != nil {
		return nil, err
	}

	conf = &Config{}
	err = yaml.NewDecoder(file).Decode(conf)
	return
}
