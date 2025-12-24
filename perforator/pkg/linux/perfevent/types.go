package perfevent

import (
	"fmt"

	"golang.org/x/sys/unix"
)

// PerfEventID is a unique identifier of a single perf event.
type PerfEventID uint64

// Type is just a handle to some underlying event,
// either perf event or some custom one.
// Particular value is assigned at runtime and should be retrieved
// upon registration or via call to `GetTypeByNameOrAlias()`.
type Type interface {
	String() string
	Name() string

	dummy()
}

type typeImpl struct {
	name string
	unit string
}

func (t *typeImpl) Name() string {
	return t.name
}

func (t *typeImpl) String() string {
	return t.name + "." + t.unit
}

func (t *typeImpl) Unit() string {
	return t.unit
}

func (t *typeImpl) dummy() {}

type PerfEventType struct {
	*typeImpl
	Type   uint32
	Config uint64
}

var (
	registeredEventTypes []Type
	nameToEventType      = make(map[string]Type)
	aliasToEventType     = make(map[string]Type)
)

func GetTypeByNameOrAlias(name string) Type {
	if typ, ok := nameToEventType[name]; ok {
		return typ
	}
	if typ, ok := aliasToEventType[name]; ok {
		return typ
	}
	return nil
}

func registerType(typ Type, aliases []string) (Type, error) {
	if _, ok := nameToEventType[typ.Name()]; ok {
		return nil, fmt.Errorf("event with name %q is already registered", typ.Name())
	}
	for _, alias := range aliases {
		if _, ok := aliasToEventType[alias]; ok {
			return nil, fmt.Errorf("event with alias %q is already registered", alias)
		}
	}
	registeredEventTypes = append(registeredEventTypes, typ)

	nameToEventType[typ.Name()] = typ
	for _, alias := range aliases {
		aliasToEventType[alias] = typ
	}
	return typ, nil
}

func mustRegisterType[T Type](typ T, aliases []string) T {
	_, err := registerType(typ, aliases)
	if err != nil {
		panic(err)
	}
	return typ
}

func getCacheConfig(id, op, result uint32) uint64 {
	return uint64(id | (op << 8) | (result << 16))
}

// See man 2 perf_event_open for the description of the event types.

var (
	CPUCycles = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "cpu", unit: "cycles"},
			Type:     unix.PERF_TYPE_HARDWARE,
			Config:   unix.PERF_COUNT_HW_CPU_CYCLES,
		},
		[]string{"CPUCycles", "cycles"},
	)
	CPUInstructions = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "instructions", unit: "count"},
			Type:     unix.PERF_TYPE_HARDWARE,
			Config:   unix.PERF_COUNT_HW_INSTRUCTIONS,
		},
		[]string{"CPUInstructions"},
	)
	CacheReferences = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "cache-references", unit: "count"},
			Type:     unix.PERF_TYPE_HARDWARE,
			Config:   unix.PERF_COUNT_HW_CACHE_REFERENCES,
		},
		[]string{"CacheReferences"},
	)
	CacheMisses = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "cache-misses", unit: "count"},
			Type:     unix.PERF_TYPE_HARDWARE,
			Config:   unix.PERF_COUNT_HW_CACHE_MISSES,
		},
		[]string{"CacheMisses"},
	)
	LLCacheLoadMisses = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "LLC-load-misses", unit: "count"},
			Type:     unix.PERF_TYPE_HW_CACHE,
			Config: getCacheConfig(
				unix.PERF_COUNT_HW_CACHE_LL,
				unix.PERF_COUNT_HW_CACHE_OP_READ,
				unix.PERF_COUNT_HW_CACHE_RESULT_MISS,
			),
		},
		[]string{"LLCacheLoadMisses"},
	)
	LLCacheStoreMisses = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "LLC-store-misses", unit: "count"},
			Type:     unix.PERF_TYPE_HW_CACHE,
			Config: getCacheConfig(
				unix.PERF_COUNT_HW_CACHE_LL,
				unix.PERF_COUNT_HW_CACHE_OP_WRITE,
				unix.PERF_COUNT_HW_CACHE_RESULT_MISS,
			),
		},
		[]string{"LLCacheStoreMisses"},
	)
	// AMD-specific hardware events
	AMDFam19hBRS = mustRegisterType(
		&PerfEventType{
			// TODO: replace with name: lbr, and unit: stacks ?
			typeImpl: &typeImpl{name: "AMDFam19hBRS"},
			// Has to be a raw event
			Type: unix.PERF_TYPE_RAW,
			// AMD-specific constant, not present in unix package
			// https://github.com/torvalds/linux/blob/07e27ad16399afcd693be20211b0dfae63e0615f/arch/x86/events/perf_event.h#L1453
			Config: 0xc4,
		},
		nil,
	)
	// Software events
	// cpu clock is broken: https://stackoverflow.com/a/56967896
	CPUClock = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "cpu-clock", unit: "seconds"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_CPU_CLOCK,
		},
		[]string{"CPUClock"},
	)
	TaskClock = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "task-clock", unit: "seconds"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_TASK_CLOCK,
		},
		[]string{"TaskClock"},
	)
	PageFaults = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "page-faults", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_PAGE_FAULTS,
		},
		[]string{"PageFaults", "faults"},
	)
	ContextSwitches = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "context-switches", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_CONTEXT_SWITCHES,
		},
		[]string{"ContextSwitches", "cs"},
	)
	CPUMigrations = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "cpu-migrations", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_CPU_MIGRATIONS,
		},
		[]string{"CPUMigrations", "migrations"},
	)
	PageFaultsMin = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "minor-faults", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_PAGE_FAULTS_MIN,
		},
		[]string{"PageFaultsMin"},
	)
	PageFaultsMaj = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "major-faults", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_PAGE_FAULTS_MAJ,
		},
		[]string{"PageFaultsMaj"},
	)
	AlignmentFaults = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "alignment-faults", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_ALIGNMENT_FAULTS,
		},
		[]string{"AlignmentFaults"},
	)
	EmulationFaults = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "emulation-faults", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_EMULATION_FAULTS,
		},
		[]string{"EmulationFaults"},
	)
	Dummy = mustRegisterType(
		&PerfEventType{
			typeImpl: &typeImpl{name: "dummy", unit: "count"},
			Type:     unix.PERF_TYPE_SOFTWARE,
			Config:   unix.PERF_COUNT_SW_DUMMY,
		},
		nil,
	)
)
