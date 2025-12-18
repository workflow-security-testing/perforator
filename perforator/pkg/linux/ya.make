GO_LIBRARY()

SRCS(
    inode_generation.go
    types.go
)

END()

RECURSE(
    btime
    cgroupfs
    cpuinfo
    cpulist
    kallsyms
    memfd
    mountinfo
    perfevent
    pidfd
    procfs
    uname
    vdso
)
