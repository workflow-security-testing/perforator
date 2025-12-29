package profiler

import (
	"github.com/yandex/perforator/perforator/agent/collector/pkg/machine/programstate"
	"github.com/yandex/perforator/perforator/pkg/linux"
)

////////////////////////////////////////////////////////////////////////////////

type trackedProcess struct {
	pid            linux.CurrentNamespacePID
	sampleConsumer SampleConsumer
	state          *programstate.State
}

func (p *Profiler) newTrackedProcess(
	pid linux.CurrentNamespacePID,
	sampleConsumer SampleConsumer,
	state *programstate.State,
) (*trackedProcess, error) {
	err := state.AddTracedProcess(pid)
	if err != nil {
		return nil, err
	}

	return &trackedProcess{
		pid:            pid,
		sampleConsumer: sampleConsumer,
		state:          state,
	}, nil
}

func (t *trackedProcess) close() error {
	return t.state.RemoveTracedProcess(t.pid)
}

////////////////////////////////////////////////////////////////////////////////
