package logfield

import (
	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/pkg/linux"
)

func CurrentNamespacePID(pid linux.CurrentNamespacePID) log.Field {
	return log.UInt32("current_namespace_pid", uint32(pid))
}

func NamespacedPID(pid linux.NamespacedPID) log.Field {
	return log.UInt32("namespaced_pid", uint32(pid))
}
