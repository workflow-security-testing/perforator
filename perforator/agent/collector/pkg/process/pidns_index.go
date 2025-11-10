package process

import (
	"sync"

	"github.com/yandex/perforator/perforator/pkg/linux"
)

var (
	_ PidNamespaceIndex = (*pidNamespaceIndex)(nil)
)

type pidNamespaceIndexKey struct {
	namespacedPID     linux.NamespacedPID
	pidNamespaceInode linux.PIDNamespaceInode
}

type PidNamespaceIndex interface {
	ResolveCurrentNamespacePID(linux.NamespacedPID, linux.PIDNamespaceInode) *linux.CurrentNamespacePID
}

// pidNamespaceIndex is a map to resolve namespaced pids into root pids (current pidns pids)
type pidNamespaceIndex struct {
	sync.RWMutex
	index map[pidNamespaceIndexKey]linux.CurrentNamespacePID
}

func newPidNamespaceIndex() *pidNamespaceIndex {
	return &pidNamespaceIndex{
		index: make(map[pidNamespaceIndexKey]linux.CurrentNamespacePID),
	}
}

func (i *pidNamespaceIndex) add(namespacedPID linux.NamespacedPID, pidNamespaceInode linux.PIDNamespaceInode, currentNamespacePID linux.CurrentNamespacePID) {
	key := pidNamespaceIndexKey{
		namespacedPID:     namespacedPID,
		pidNamespaceInode: pidNamespaceInode,
	}

	i.Lock()
	defer i.Unlock()
	i.index[key] = currentNamespacePID
}

func (i *pidNamespaceIndex) remove(namespacedPID linux.NamespacedPID, pidNamespaceInode linux.PIDNamespaceInode) {
	key := pidNamespaceIndexKey{
		namespacedPID:     namespacedPID,
		pidNamespaceInode: pidNamespaceInode,
	}

	i.Lock()
	defer i.Unlock()
	delete(i.index, key)
}

func (i *pidNamespaceIndex) ResolveCurrentNamespacePID(namespacedPID linux.NamespacedPID, pidNamespaceInode linux.PIDNamespaceInode) *linux.CurrentNamespacePID {
	key := pidNamespaceIndexKey{
		namespacedPID:     namespacedPID,
		pidNamespaceInode: pidNamespaceInode,
	}
	i.RLock()
	defer i.RUnlock()
	pid, ok := i.index[key]
	if !ok {
		return nil
	}

	return &pid
}
