package render

import (
	"io"

	pprof "github.com/google/pprof/profile"

	"github.com/yandex/perforator/perforator/pkg/profile/flamegraph/collapsed"
)

// FlameGraphRenderer renders flamegraphs from profiles.
// This interface is implemented by both the Go blocks-based renderer (FlameGraph)
// and the C++ trie-based renderer (CGOFlameGraph).
type FlameGraphRenderer interface {
	// Profile input
	AddProfile(p *pprof.Profile) error
	AddBaselineProfile(p *pprof.Profile) error
	AddCollapsedProfile(p *collapsed.Profile) error
	AddCollapsedBaselineProfile(p *collapsed.Profile) error

	// Display options
	SetFormat(format Format)
	SetTitle(title string)
	SetInverted(value bool)
	SetMinWeight(value float64)
	SetDepthLimit(value int)
	SetSampleType(typ string)

	// Location display options
	SetLineNumbers(value bool)
	SetFileNames(value bool)
	SetFilePathPrefix(value string)
	SetAddressRenderPolicy(policy AddressRenderPolicy)
	SetIgnoreFullPath(value bool)

	// Output
	Render(w io.Writer) error
	TotalEvents() float64
}
