package process

import (
	"github.com/yandex/perforator/perforator/agent/collector/pkg/dso"
	"github.com/yandex/perforator/perforator/pkg/linux"
	"github.com/yandex/perforator/perforator/pkg/xelf"
)

type Mapping interface {
	Path() string
	dso() *dso.DSO
	begin() uint64
	end() uint64
	buildInfo() *xelf.BuildInfo
}

type ProcessInfo interface {
	ProcessID() linux.CurrentNamespacePID
	// returned map may not be modified
	Env() map[string]string
	// returned slice may not be modified
	Mappings() []Mapping
}

type Listener interface {
	OnProcessDiscovery(info ProcessInfo)
	OnProcessRescan(info ProcessInfo)
	OnProcessDeath(pid linux.CurrentNamespacePID)
}
