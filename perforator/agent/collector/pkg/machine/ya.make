GO_LIBRARY()

SRCS(
    bpf.go
    links.go
    pin.go
)

END()

RECURSE(
    programstate
)
