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

type DaemonConfig struct {
	Level         log.Level `yaml:"level"`
	Format        LogFormat `yaml:"format"`
	EnableMetrics bool      `yaml:"enable_metrics"`
}

func ForDaemon(cfg DaemonConfig, metrics metrics.Registry) (Logger, func(), error) {
	var fields []zapcore.Field

	var encoder zapcore.Encoder
	switch cfg.Format {
	case LogFormatUnspecified:
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
	core := asynczap.NewCore(encoder, zapcore.Lock(os.Stdout), zap.ZapifyLevel(cfg.Level), asynczap.Options{
		FlushInterval: time.Second,
	})

	var logger log.Logger
	logger = zap.NewWithCore(core.With(fields))
	if cfg.EnableMetrics {
		logger = logmetrics.NewMeteredLogger(logger, metrics)
		// TODO: metrics for async core itself
	}
	return Wrap(logger), core.Stop, nil
}
