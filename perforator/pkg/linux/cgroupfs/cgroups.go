package cgroupfs

import (
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/yandex/perforator/perforator/pkg/linux"
)

const (
	CgroupFSPathPrefix = "/sys/fs/cgroup"
)

func GetCgroupV2Pids(cgroupPath string) ([]linux.CurrentNamespacePID, error) {
	procsFile := filepath.Join(CgroupFSPathPrefix, cgroupPath, "cgroup.procs")
	return readPidsFromProcsFile(procsFile)
}

func GetCgroupV1Pids(controller, cgroupPath string) ([]linux.CurrentNamespacePID, error) {
	procsFile := filepath.Join(CgroupFSPathPrefix, controller, cgroupPath, "cgroup.procs")
	return readPidsFromProcsFile(procsFile)
}

func readPidsFromProcsFile(path string) ([]linux.CurrentNamespacePID, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	lines := strings.Split(string(data), "\n")
	var pids []linux.CurrentNamespacePID
	for _, line := range lines {
		if line == "" {
			continue
		}
		pid, err := strconv.Atoi(strings.TrimSpace(line))
		if err == nil {
			pids = append(pids, linux.CurrentNamespacePID(pid))
		}
	}
	return pids, nil
}
