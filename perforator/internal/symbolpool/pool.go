package symbolpool

import (
	"slices"
	"sync"
	"sync/atomic"

	"github.com/yandex/perforator/perforator/pkg/linux"
)

type Symbol struct {
	Name  string
	Begin uint64
	Size  uint64
}

type process struct {
	current atomic.Pointer[[]Symbol]
}

// Pool is a simple container which stores a set of symbols (i.e. address ranges)
// belonging to different processes. It can then find a symbol, containing specified address
type Pool struct {
	mu    sync.RWMutex
	procs map[linux.CurrentNamespacePID]*process
}

func New() *Pool {
	return &Pool{
		procs: make(map[linux.CurrentNamespacePID]*process),
	}
}

func (p *process) reset(newSyms []Symbol) {
	if p == nil {
		panic("process is nil")
	}
	p.current.Store(&newSyms)
}

// Put replaces current set of symbols for process pid with newSyms.
// It expects that newSyms is pairwise disjoint and sorted on Begin.
func (p *Pool) Put(pid linux.CurrentNamespacePID, newSyms []Symbol) {
	var proc *process
	{
		p.mu.RLock()
		proc = p.procs[pid]
		if proc != nil {
			proc.reset(newSyms)
		}
		p.mu.RUnlock()
	}
	if proc != nil {
		return
	}
	p.mu.Lock()
	proc = p.procs[pid]
	if proc == nil {
		proc = new(process)
		p.procs[pid] = proc
	}
	proc.reset(newSyms)
	p.mu.Unlock()
}

func (p *Pool) Remove(pid linux.CurrentNamespacePID) {
	p.mu.Lock()
	delete(p.procs, pid)
	p.mu.Unlock()
}

func (p *Pool) Resolve(pid linux.CurrentNamespacePID, ip uint64) (string, bool) {
	if p == nil {
		panic("receiver is nil")
	}
	var syms []Symbol
	{
		p.mu.RLock()
		proc := p.procs[pid]
		if proc != nil {
			syms = *proc.current.Load()
		}
		p.mu.RUnlock()
	}
	pos, ok := slices.BinarySearchFunc(syms, ip, func(s Symbol, target uint64) int {
		if target < s.Begin {
			return -1
		}
		if target >= s.Begin+s.Size {
			return 1
		}
		return 0
	})
	if !ok {
		return "", false
	}
	return syms[pos].Name, true
}
