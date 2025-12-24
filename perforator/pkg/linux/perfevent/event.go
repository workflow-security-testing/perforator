package perfevent

import (
	"fmt"
	"os"
	"syscall"
	"unsafe"

	"golang.org/x/sys/unix"

	"github.com/yandex/perforator/perforator/pkg/linux/uname"
)

////////////////////////////////////////////////////////////////////////////////

func ShouldTryToEnableBranchSampling() bool {
	// The `bpf_read_branch_records` function we use (if available) in eBPF side of the agent
	// is introduced in 5.7 kernel. Without this functionality in eBPF, sampling branches is a
	// pure overhead and should be avoided.
	releaseVersion, err := uname.SystemRelease()
	if err != nil {
		return false
	}

	majorMinor := []int{0, 0}
	idx := 0

	for i := 0; i < len(releaseVersion); i++ {
		c := releaseVersion[i]
		if c >= '0' && c <= '9' {
			majorMinor[idx] = majorMinor[idx]*10 + int(c-'0')
		} else {
			idx += 1
			if idx >= len(majorMinor) {
				break
			}
		}
	}
	major, minor := majorMinor[0], majorMinor[1]

	return major > 5 || (major == 5 && minor >= 7)
}

////////////////////////////////////////////////////////////////////////////////

type Target struct {
	// ID of the process to trace.
	ProcessID *int

	// FD of the cgroup to trace.
	CgroupFD *int

	// If set, trace whole system.
	WholeSystem bool

	// ID of the CPU core to trace.
	// If CPU is nil, trace every core.
	CPU *int
}

type Options struct {
	// Type of the perf event.
	Type *PerfEventType

	// Event sampling rate.
	SampleRate *uint64

	// Number of events per second, HZ.
	// The kernel will try to select sampling rate to match the requested frequency.
	Frequency *uint64

	// Pin events on the CPU.
	Pinned bool

	// Create perf event enabled by default.
	Enable bool

	// Try to enable branch sampling (PERF_SAMPLE_BRANCH_STACK)
	TryToSampleBranchStack bool
}

type branchStackOptions struct {
	// Sample branch stack
	Enable bool
}

type Handle struct {
	fd *os.File
	id PerfEventID
}

// TODO(sskvor): Use less hardcoded values, add more options
func NewHandle(target *Target, options *Options) (*Handle, error) {
	if options.TryToSampleBranchStack {
		// Branch stack sampling could be not supported by either hardware, kernel or event type.
		// We try to create a perf event with it for LBR collection,
		// but if it fails we just fall back to not using it.
		handle, err := newHandle(target, options, branchStackOptions{Enable: true})
		if err == nil {
			return handle, nil
		}
	}

	return newHandle(target, options, branchStackOptions{Enable: false})
}

// Close this perf event. Stop generating samples, detach attached eBPF programs.
func (h *Handle) Close() error {
	return h.fd.Close()
}

// Start generating events by this perf event.
// New events by default are enabled.
func (h *Handle) Enable() error {
	return unix.IoctlSetInt(int(h.fd.Fd()), unix.PERF_EVENT_IOC_ENABLE, 1)
}

// Stop generating events by this perf event.
func (h *Handle) Disable() error {
	return unix.IoctlSetInt(int(h.fd.Fd()), unix.PERF_EVENT_IOC_DISABLE, 1)
}

// Attach existing eBPF program to the perf event.
func (h *Handle) AttachBPF(bpfFD int) error {
	return unix.IoctlSetInt(int(h.fd.Fd()), unix.PERF_EVENT_IOC_SET_BPF, bpfFD)
}

// Get system-wide unique id of the perf event.
func (h *Handle) ID() PerfEventID {
	return h.id
}

func getPerfEventID(fd *os.File) (id PerfEventID, err error) {
	var rawID uint64
	_, _, errno := unix.Syscall(syscall.SYS_IOCTL, fd.Fd(), unix.PERF_EVENT_IOC_ID, uintptr(unsafe.Pointer(&rawID)))
	if errno != 0 {
		err = errno
		return
	}
	return PerfEventID(rawID), nil
}

func newHandle(target *Target, options *Options, branchStackOptions branchStackOptions) (*Handle, error) {
	attr, err := makePerfEventAttr(options, &branchStackOptions)
	if err != nil {
		return nil, fmt.Errorf("failed to prepare perf event attributes: %w", err)
	}

	h, err := newHandleFromAttr(attr, target)
	if err != nil {
		return nil, fmt.Errorf("failed to create perf event: %w", err)
	}
	return h, nil
}

func newHandleFromAttr(attr *unix.PerfEventAttr, target *Target) (hndl *Handle, err error) {
	flags := 0
	flags |= unix.PERF_FLAG_FD_CLOEXEC

	pid := -1
	if target.ProcessID != nil {
		pid = *target.ProcessID
	} else if target.CgroupFD != nil {
		pid = *target.CgroupFD
		flags |= unix.PERF_FLAG_PID_CGROUP
	}

	cpu := -1
	if target.CPU != nil {
		cpu = *target.CPU
	}

	fd, err := unix.PerfEventOpen(attr, pid, cpu, -1 /*groupFd*/, flags)
	if err != nil {
		return nil, fmt.Errorf("syscall perf_event_open(attr=%+v, pid=%d, cpu=%d, flags=%d) failed: %w", attr, pid, cpu, flags, err)
	}

	file := os.NewFile(uintptr(fd), fmt.Sprintf("perf_event_open/%d/%d", pid, cpu))
	defer func() {
		if err != nil {
			_ = file.Close()
		}
	}()

	id, err := getPerfEventID(file)
	if err != nil {
		return nil, fmt.Errorf("failed to get perf event id: %w", err)
	}

	return &Handle{file, id}, nil
}

func makePerfEventAttr(options *Options, branchStackOptions *branchStackOptions) (*unix.PerfEventAttr, error) {
	attr := &unix.PerfEventAttr{}
	attr.Size = uint32(unsafe.Sizeof(*attr))

	attr.Type = options.Type.Type
	attr.Config = options.Type.Config

	attr.Sample_type = 0 |
		unix.PERF_SAMPLE_CALLCHAIN |
		unix.PERF_SAMPLE_IP |
		unix.PERF_SAMPLE_TID

	if branchStackOptions.Enable {
		attr.Sample_type |= unix.PERF_SAMPLE_BRANCH_STACK
		attr.Branch_sample_type = 0 |
			unix.PERF_SAMPLE_BRANCH_USER |
			unix.PERF_SAMPLE_BRANCH_ANY
	}

	if options.Frequency != nil {
		attr.Sample = *options.Frequency
		attr.Bits |= unix.PerfBitFreq
	} else if options.SampleRate != nil {
		attr.Sample = *options.SampleRate
	} else {
		return nil, fmt.Errorf("no Frequency or SampleRate is set")
	}

	if options.Pinned {
		attr.Bits |= unix.PerfBitPinned
	}

	if !options.Enable {
		attr.Bits |= unix.PerfBitDisabled
	}

	return attr, nil
}
