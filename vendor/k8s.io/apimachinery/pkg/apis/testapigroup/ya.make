GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.31.6)

SRCS(
    doc.go
    register.go
    types.go
    zz_generated.deepcopy.go
)

END()

RECURSE(
    fuzzer
    install
    v1
)
