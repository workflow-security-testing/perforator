package profiler

import (
	"context"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/agent/collector/pkg/uprobe"
	"github.com/yandex/perforator/perforator/internal/unwinder"
	"github.com/yandex/perforator/perforator/pkg/linux"
)

////////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns true if the sample should be consumed
// Returns false if the sample should be filtered out.
type SampleFilterFunc func(ctx context.Context, sample *unwinder.RecordSample) bool

func NewUprobeSampleFilter(p *Profiler, allowedUprobes map[uprobe.BinaryInfo]struct{}) SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		if sample.SampleType != unwinder.SampleTypeUprobe {
			return false
		}

		topStackIP := sample.Userstack[0]
		if topStackIP == 0 {
			return false
		}

		mapping, err := p.dsoStorage.ResolveMapping(ctx, linux.CurrentNamespacePID(sample.Pid), topStackIP)
		if err != nil {
			p.log.Warn("Failed to resolve mapping for uprobe sample", log.UInt64("top_stack_ip", topStackIP), log.Error(err))
			return false
		}

		// Sanity check, this must never happen
		if mapping.BuildInfo == nil {
			p.log.Error("No build info for resolved mapping", log.UInt64("top_stack_ip", topStackIP))
			return false
		}

		_, allowUprobe := allowedUprobes[uprobe.BinaryInfo{
			Offset:  topStackIP - mapping.Begin + mapping.Offset,
			BuildID: mapping.BuildInfo.BuildID,
		}]
		return allowUprobe
	}
}

func NewNonUprobeSampleFilter() SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		return sample.SampleType != unwinder.SampleTypeUprobe
	}
}

func NewPIDSampleFilter(pid linux.CurrentNamespacePID) SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		return linux.CurrentNamespacePID(sample.Pid) == pid
	}
}

func NewTIDSampleFilter(tid linux.CurrentNamespacePID) SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		return linux.CurrentNamespacePID(sample.Tid) == tid
	}
}

func NewPIDOrTIDSampleFilter(pid linux.CurrentNamespacePID) SampleFilterFunc {
	return NewORSampleFilter(NewPIDSampleFilter(pid), NewTIDSampleFilter(pid))
}

func NewORSampleFilter(filters ...SampleFilterFunc) SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		for _, filter := range filters {
			if filter(ctx, sample) {
				return true
			}
		}

		return false
	}
}

func NewANDSampleFilter(filters ...SampleFilterFunc) SampleFilterFunc {
	return func(ctx context.Context, sample *unwinder.RecordSample) bool {
		for _, filter := range filters {
			if !filter(ctx, sample) {
				return false
			}
		}

		return true
	}
}

type filterSampleConsumerAdapter struct {
	filter       SampleFilterFunc
	baseConsumer SampleConsumer
}

// if all of the filters returns true the sample will be consumed
func NewFilterSampleConsumerAdapter(
	baseConsumer SampleConsumer,
	filters ...SampleFilterFunc,
) *filterSampleConsumerAdapter {
	return &filterSampleConsumerAdapter{
		baseConsumer: baseConsumer,
		filter:       NewANDSampleFilter(filters...),
	}
}

func (c *filterSampleConsumerAdapter) Consume(ctx context.Context, sample *unwinder.RecordSample) {
	if !c.filter(ctx, sample) {
		return
	}

	c.baseConsumer.Consume(ctx, sample)
}

func (c *filterSampleConsumerAdapter) Flush(ctx context.Context) error {
	return c.baseConsumer.Flush(ctx)
}
