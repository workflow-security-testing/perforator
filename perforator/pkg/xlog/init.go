package xlog

import (
	"fmt"
	"os"
	"time"

	"github.com/mattn/go-isatty"
	uberzap "go.uber.org/zap"
	"go.uber.org/zap/zapcore"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/log/zap"
	"github.com/yandex/perforator/library/go/core/log/zap/asynczap"
	"github.com/yandex/perforator/library/go/core/log/zap/encoders"
	"github.com/yandex/perforator/library/go/core/metrics"
	"github.com/yandex/perforator/perforator/pkg/xlog/logmetrics"
)

type LogFormat int

const (
	// text for CLI, json for daemons
	LogFormatUnspecified LogFormat = iota
	// only supported for CLI
	LogFormatText
	LogFormatJson
	// only supported for daemons
	LogFormatTSKV
)

type CLIConfig struct {
	Format LogFormat
	Level  log.Level
}

func ForCLI(cfg CLIConfig) (Logger, error) {
	var logger *zap.Logger
	var err error
	switch cfg.Format {
	case LogFormatJson:
		logger, err = zap.NewDeployLogger(cfg.Level)
	case LogFormatUnspecified:
		fallthrough
	case LogFormatText:
		config := uberzap.NewDevelopmentConfig()
		config.Level = uberzap.NewAtomicLevelAt(zap.ZapifyLevel(cfg.Level))

		if isatty.IsTerminal(os.Stderr.Fd()) {
			config.EncoderConfig.EncodeLevel = zapcore.CapitalColorLevelEncoder
		} else {
			config.EncoderConfig.EncodeLevel = zapcore.CapitalLevelEncoder
		}

		config.EncoderConfig.ConsoleSeparator = " "
		config.EncoderConfig.EncodeTime = zapcore.TimeEncoderOfLayout(`15:04:05.000`)
		config.DisableStacktrace = true
		logger, err = zap.New(config)
	default:
		return nil, fmt.Errorf("unsupported format: %v", cfg.Format)
	}

	if err != nil {
		return nil, fmt.Errorf("failed to initialize logging: %w", err)
	}
	return Wrap(logger), nil
}

type SamplingConfig struct {
	Enabled bool `yaml:"enabled"`

	// Initial and Thereafter configure log sampling. Semantically, they work as follows:
	// Every second, all log messages logged during that second are grouped into sets of
	// similar messages.
	// Then, for each group:
	// - First Initial are logged
	// - Then, if Thereafter is greater than 0, each (1/Thereafter)-th message will be logged
	// - All remaining messages in the group are dropped.

	// Initial configures log sampling. See above for meaning.
	Initial int `yaml:"initial"`

	// Thereafter configures log sampling. See above for meaning.
	Thereafter    int   `yaml:"thereafter"`
	EnableMetrics *bool `yaml:"enable_metrics"`
}

type DaemonConfig struct {
	Level         log.Level      `yaml:"level"`
	Format        LogFormat      `yaml:"format"`
	EnableMetrics bool           `yaml:"enable_metrics"`
	Sampling      SamplingConfig `yaml:"sampling"`
}

func ForDaemon(cfg DaemonConfig, metrics metrics.Registry) (Logger, func(), error) {
	var fields []zapcore.Field

	var encoder zapcore.Encoder
	switch cfg.Format {
	case LogFormatUnspecified:
		fallthrough
	case LogFormatJson:
		encoderconf := zap.NewDeployEncoderConfig()
		fields = append(fields, uberzap.Namespace("@fields"))
		encoder = zapcore.NewJSONEncoder(encoderconf)
	case LogFormatTSKV:
		encoderconf := uberzap.NewProductionEncoderConfig()
		encoderconf.EncodeTime = zapcore.RFC3339NanoTimeEncoder
		var err error
		encoder, err = encoders.NewTSKVEncoder(encoderconf)
		if err != nil {
			return nil, nil, fmt.Errorf("failed to initialize tskv encoder: %w", err)
		}
	default:
		return nil, nil, fmt.Errorf("unsupported format: %v", cfg.Format)
	}
	asyncCore := asynczap.NewCore(encoder, zapcore.Lock(os.Stdout), zap.ZapifyLevel(cfg.Level), asynczap.Options{
		FlushInterval: time.Second,
	})
	core := asyncCore.With(fields)
	if cfg.Sampling.Enabled {
		// same as zap
		samplingTick := time.Second

		var opts []zapcore.SamplerOption
		if cfg.Sampling.EnableMetrics == nil || *cfg.Sampling.EnableMetrics {
			counter := metrics.Counter("log.sampling.drops")
			opts = append(opts,
				zapcore.SamplerHook(func(entry zapcore.Entry, dec zapcore.SamplingDecision) {
					if dec&zapcore.LogDropped != 0 {
						counter.Inc()
					}
				}))
		}
		core = zapcore.NewSamplerWithOptions(
			core,
			samplingTick,
			cfg.Sampling.Initial,
			cfg.Sampling.Thereafter,
			opts...,
		)
	}

	var logger log.Logger
	logger = zap.NewWithCore(core)
	if cfg.EnableMetrics {
		logger = logmetrics.NewMeteredLogger(logger, metrics)
		// TODO: metrics for async core itself
	}
	return Wrap(logger), asyncCore.Stop, nil
}
