package samplefilter

import (
	"testing"

	pprof "github.com/google/pprof/profile"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"

	"github.com/yandex/perforator/perforator/pkg/profilequerylang"
)

func TestBuildBuildIDFilter(t *testing.T) {
	for _, test := range []struct {
		name   string
		query  string
		error  bool
		filter *buildIDFilter
	}{
		{
			name:   "EmptySelector",
			query:  "{}",
			filter: &buildIDFilter{buildIDs: map[string]struct{}{}},
		},
		{
			name:   "UnrelatedMatchers",
			query:  "{env.foo=\"123\"}",
			filter: &buildIDFilter{buildIDs: map[string]struct{}{}},
		},
		{
			name:   "UnsupportedMultiValue",
			query:  "{build_ids=\"123|456\"}",
			filter: &buildIDFilter{buildIDs: map[string]struct{}{"123": {}, "456": {}}},
		},
		{
			name:  "UnsupportedNotEqual",
			query: "{build_ids!=\"123\"}",
			error: true,
		},
		{
			name:   "OK",
			query:  "{build_ids=\"123\"}",
			filter: &buildIDFilter{buildIDs: map[string]struct{}{"123": {}}},
		},
	} {
		t.Run(test.name, func(t *testing.T) {
			sel, err := profilequerylang.ParseSelector(test.query)
			require.NoError(t, err)
			f, err := BuildBuildIDFilter(sel)
			if test.error {
				assert.Error(t, err)
				return
			} else {
				if assert.NoError(t, err) {
					assert.Equal(t, test.filter, f)
				}
			}
		})
	}
}

func TestBuildIDFilterMatch(t *testing.T) {
	tests := []struct {
		name     string
		filter   *buildIDFilter
		buildIDs []string
		expected bool
	}{
		{
			name:     "EmptyFilterMatchesAnyBuildID",
			filter:   &buildIDFilter{buildIDs: map[string]struct{}{}},
			buildIDs: []string{"123", "456"},
			expected: true,
		},
		{
			name:     "EmptyFilterMatchesMissingBuildID",
			filter:   &buildIDFilter{buildIDs: map[string]struct{}{}},
			buildIDs: []string{"123"},
			expected: true,
		},
		{
			name:     "SimpleMatch",
			filter:   &buildIDFilter{buildIDs: map[string]struct{}{"123": {}}},
			buildIDs: []string{"123"},
			expected: true,
		},
		{
			name:     "SimpleNoMatch",
			filter:   &buildIDFilter{buildIDs: map[string]struct{}{"123": {}}},
			buildIDs: []string{"456"},
			expected: false,
		},
		{
			name:     "MatchOneOf",
			filter:   &buildIDFilter{buildIDs: map[string]struct{}{"123": {}, "456": {}}},
			buildIDs: []string{"456"},
			expected: true,
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			s := &pprof.Sample{}
			for _, b := range test.buildIDs {
				s.Location = append(s.Location, &pprof.Location{
					Mapping: &pprof.Mapping{
						BuildID: b,
					},
				})
			}
			assert.Equal(t, test.expected, test.filter.Matches(s))
		})
	}
}
