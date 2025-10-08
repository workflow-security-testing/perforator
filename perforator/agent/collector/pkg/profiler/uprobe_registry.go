package profiler

import (
	"errors"
	"fmt"
	"sync"

	"github.com/yandex/perforator/perforator/agent/collector/pkg/machine"
	"github.com/yandex/perforator/perforator/agent/collector/pkg/uprobe"
)

// uprobeRegistry is a container which stores all the created uprobes.
// It provides these functions:
// 1. Create uprobe
// 2. Resolve profiler samples into specific uprobes
// 3. Reattach uprobes on bpf program reload (e.g. after SetDebugMode)
//
// uprobeRegistry assumes all uprobes are linked to the same single bpf program.
type uprobeRegistry struct {
	sync.Mutex
	id      uint64
	uprobes map[uint64]*uprobeWrapper
	*uprobe.Resolver
	bpf *machine.BPF
}

func newUprobeRegistry(bpf *machine.BPF) *uprobeRegistry {
	return &uprobeRegistry{
		id:       0,
		uprobes:  make(map[uint64]*uprobeWrapper),
		Resolver: uprobe.NewResolver(),
		bpf:      bpf,
	}
}

func (r *uprobeRegistry) detachAll() error {
	r.Lock()
	defer r.Unlock()

	var errs []error
	for _, uprobe := range r.uprobes {
		err := uprobe.detach()
		if err != nil {
			errs = append(errs, err)
		}
	}

	return errors.Join(errs...)
}

func (r *uprobeRegistry) attachAll() error {
	r.Lock()
	defer r.Unlock()

	var errs []error
	for _, uprobe := range r.uprobes {
		err := uprobe.attach()
		if err != nil {
			errs = append(errs, err)
		}
	}

	return errors.Join(errs...)
}

func (r *uprobeRegistry) register(uprobe *uprobeWrapper) (id uint64) {
	r.Lock()
	defer r.Unlock()

	r.uprobes[r.id] = uprobe
	r.id++

	return r.id
}

func (r *uprobeRegistry) unregister(id uint64) {
	r.Lock()
	defer r.Unlock()

	_, ok := r.uprobes[id]
	if !ok {
		panic("unregistering uprobe which is not in a registry")
	}

	delete(r.uprobes, id)
}

type uprobeWrapper struct {
	uprobe     uprobe.Uprobe
	attached   bool
	registryID uint64
	registry   *uprobeRegistry
}

func (u *uprobeWrapper) attach() error {
	program := u.registry.bpf.GenericUprobeProgram()
	if program == nil {
		// sanity check
		return errors.New("generic uprobe program is not loaded")
	}

	err := u.uprobe.Attach(program)
	if err != nil {
		return err
	}

	u.attached = true
	u.registry.Resolver.Add(u.uprobe.Info())

	return nil
}

func (u *uprobeWrapper) detach() error {
	if !u.attached {
		return nil
	}

	binaryInfo := u.uprobe.Info().BinaryInfo
	err := u.uprobe.Close()
	if err != nil {
		return err
	}

	u.attached = false
	u.registry.Resolver.Remove(binaryInfo)
	return nil
}

func (u *uprobeWrapper) close() error {
	err := u.detach()
	if err != nil {
		return fmt.Errorf("failed to detach uprobe: %w", err)
	}

	u.registry.unregister(u.registryID)
	return nil
}

func (r *uprobeRegistry) create(config uprobe.Config) *uprobeWrapper {
	uprobe := &uprobeWrapper{
		uprobe:   uprobe.NewUprobe(config),
		registry: r,
	}

	uprobe.registryID = r.register(uprobe)
	return uprobe
}
