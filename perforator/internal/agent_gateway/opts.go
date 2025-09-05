package agent_gateway

import (
	"os"
	"time"
)

type options struct {
	storage storageOptions
}

func defaultOpts() *options {
	return &options{
		storage: storageOptions{
			clusterName:            os.Getenv("DEPLOY_NODE_DC"),
			samplingModulo:         1,
			maxBuildIDCacheEntries: 14000000,
			pushProfileTimeout:     10 * time.Second,
			pushBinaryWriteAbility: true,
		},
	}
}

type Option func(*options)

func WithClusterName(clusterName string) Option {
	return func(o *options) {
		o.storage.clusterName = clusterName
	}
}

func WithSamplingModulo(samplingModulo uint64) Option {
	return func(o *options) {
		o.storage.samplingModulo = samplingModulo
	}
}

func WithSamplingModuloByEvent(samplingModuloByEvent map[string]uint64) Option {
	return func(o *options) {
		o.storage.samplingModuloByEvent = samplingModuloByEvent
	}
}
func WithMaxBuildIDCacheEntries(maxBuildIDCacheEntries uint64) Option {
	return func(o *options) {
		o.storage.maxBuildIDCacheEntries = maxBuildIDCacheEntries
	}
}

func WithPushProfileTimeout(pushProfileTimeout time.Duration) Option {
	return func(o *options) {
		o.storage.pushProfileTimeout = pushProfileTimeout
	}
}

func WithPushBinaryWriteAbility(pushBinaryWriteAbility bool) Option {
	return func(o *options) {
		o.storage.pushBinaryWriteAbility = pushBinaryWriteAbility
	}
}
