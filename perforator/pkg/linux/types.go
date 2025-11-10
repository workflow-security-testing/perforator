package linux

type processID uint32

// ProcessID in the pid namespace of the current process
type CurrentNamespacePID processID

// ProcessID in the pid namespace of the target process
type NamespacedPID processID

type PIDNamespaceInode uint64
