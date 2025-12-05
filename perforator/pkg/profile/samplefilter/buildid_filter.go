package samplefilter

import (
	"fmt"

	pprof "github.com/google/pprof/profile"

	"github.com/yandex/perforator/observability/lib/querylang"
	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
	profilepb "github.com/yandex/perforator/perforator/proto/profile"
)

type buildIDFilter struct {
	buildIDs map[string]struct{}
}

const (
	buildIDMatcherField string = "build_ids"
)

func (bf *buildIDFilter) Matches(sample *pprof.Sample) bool {
	if len(bf.buildIDs) == 0 {
		return true
	}
	for _, location := range sample.Location {
		if location.Mapping == nil {
			continue
		}
		if _, ok := bf.buildIDs[location.Mapping.BuildID]; ok {
			return true
		}
	}
	return false
}

func (bf *buildIDFilter) AppendToProto(filter *profilepb.SampleFilter) {
	for buildID := range bf.buildIDs {
		filter.RequiredOneOfBuildIds = append(filter.RequiredOneOfBuildIds, buildID)
	}
}

func BuildBuildIDFilter(selector *querylang.Selector) (SampleFilter, error) {
	filter := &buildIDFilter{
		buildIDs: make(map[string]struct{}),
	}

	for _, matcher := range selector.Matchers {
		if matcher.Field != buildIDMatcherField {
			continue
		}
		values, err := profilequerylang.ExtractEqualityMatch(matcher)
		if err != nil {
			return nil, fmt.Errorf("failed to extract desired build id: %w", err)
		}
		if len(filter.buildIDs) != 0 {
			return nil, fmt.Errorf("multiple build_ids filters are not supported")
		}
		filter.buildIDs = values
	}

	return filter, nil
}
