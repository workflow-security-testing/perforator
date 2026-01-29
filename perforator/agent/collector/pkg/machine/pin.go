package machine

import (
	"errors"
	"fmt"
	"os"
	"path"

	"github.com/cilium/ebpf"

	"github.com/yandex/perforator/perforator/internal/unwinder"
)

func (b *BPF) pinMap(mp *ebpf.Map, name string) error {
	path := path.Join(b.conf.BPFFSRoot, fmt.Sprintf("%s%s", b.conf.PinPrefix, name))
	err := os.Remove(path)
	if err != nil && !errors.Is(err, os.ErrNotExist) {
		return fmt.Errorf("failed to unpin old %s map: %w", name, err)
	}
	err = mp.Pin(path)
	if err != nil {
		return fmt.Errorf("failed to pin %s map: %w", name, err)
	}
	b.mapsToUnpin = append(b.mapsToUnpin, mp)
	return nil
}

func (b *BPF) pinIfNeeded(maps *unwinder.Maps) error {
	if !b.opts.EnableJVM {
		return nil
	}
	if b.conf.PinPrefix == "" {
		return fmt.Errorf("internal error: pinning is needed, but no pin_prefix was configured")
	}
	err := b.pinMap(maps.ProcessInfo, "process_info")
	if err != nil {
		return err
	}
	return nil
}
