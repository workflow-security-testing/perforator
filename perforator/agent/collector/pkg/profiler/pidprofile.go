package profiler

import (
	"github.com/yandex/perforator/perforator/agent/collector/pkg/machine"
	"github.com/yandex/perforator/perforator/pkg/linux"
)

////////////////////////////////////////////////////////////////////////////////

type trackedProcess struct {
	pid            linux.CurrentNamespacePID
	sampleConsumer SampleConsumer
	bpf            *machine.BPF
}

func (p *Profiler) newTrackedProcess(
	pid linux.CurrentNamespacePID,
	sampleConsumer SampleConsumer,
	bpf *machine.BPF,
) (*trackedProcess, error) {
	err := bpf.AddTracedProcess(pid)
	if err != nil {
		return nil, err
	}

	return &trackedProcess{
		pid:            pid,
		sampleConsumer: sampleConsumer,
		bpf:            bpf,
	}, nil
}

func (t *trackedProcess) close() error {
	return t.bpf.RemoveTracedProcess(t.pid)
}

////////////////////////////////////////////////////////////////////////////////
