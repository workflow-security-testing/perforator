GO_LIBRARY()

SRCS(
    inode_generation.go
    types.go
)

END()

RECURSE(
    btime
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
