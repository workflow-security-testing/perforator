package programstate

import (
	"fmt"

	"github.com/cilium/ebpf"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/internal/unwinder"
)

func (s *State) UnwindTablePartCount() int {
	return s.unwindTablePartCount
}

func getPartID(pageID unwinder.PageId) uint32 {
	return uint32(pageID) / uint32(unwinder.UnwindPageTableNumPagesPerPart)
}

func getPartPageID(pageID unwinder.PageId) uint32 {
	return uint32(pageID) % uint32(unwinder.UnwindPageTableNumPagesPerPart)
}

func (s *State) getUnwindTablePart(partID uint32) *ebpf.Map {
	s.partsmu.RLock()
	defer s.partsmu.RUnlock()
	return s.unwindTableParts[partID]
}

func (s *State) getOrInsertUnwindTablePart(partID uint32) (*ebpf.Map, error) {
	part := s.getUnwindTablePart(partID)
	if part != nil {
		return part, nil
	}
	s.partsmu.Lock()
	defer s.partsmu.Unlock()
	if s.unwindTableParts[partID] != nil {
		return s.unwindTableParts[partID], nil
	}

	partSpec := s.unwindTablePartSpec.Copy()
	var err error
	part, err = ebpf.NewMap(partSpec)
	if err != nil {
		return nil, fmt.Errorf("failed to create new unwind table part: %w", err)
	}
	partFD := uint32(part.FD())
	s.logger.Debug("Allocated new unwind table part", log.UInt32("part_id", partID), log.UInt32("part_fd", partFD))
	err = s.maps.UnwindTable.Put(&partID, &partFD)
	if err != nil {
		cleanupErr := part.Close()
		if cleanupErr != nil {
			s.logger.Warn("Failed to cleanup unwind table part after failed insert, it will be leaked", log.Error(cleanupErr))
		}
		return nil, fmt.Errorf("failed to add part into unwind table: %w", err)
	}
	s.unwindTableParts[partID] = part
	partCount := len(s.unwindTableParts)
	s.currentPartCount.Set(int64(partCount))
	// TODO: track page count more accurately
	s.currentPageCount.Set(int64(partCount) * int64(unwinder.UnwindPageTableNumPagesPerPart))
	return part, nil
}

func (s *State) PutUnwindTablePage(id unwinder.PageId, page *unwinder.UnwindTablePage) error {
	part, err := s.getOrInsertUnwindTablePart(getPartID(id))
	if err != nil {
		return fmt.Errorf("failed to get or insert part (id=%d): %w", getPartID(id), err)
	}
	partPageID := getPartPageID(id)
	err = part.Put(&partPageID, page)
	if err != nil {
		return fmt.Errorf("failed to add page into part (id=%d): %w", getPartID(id), err)
	}
	return nil
}
