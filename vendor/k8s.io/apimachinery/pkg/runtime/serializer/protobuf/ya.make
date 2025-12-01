GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.31.6)

SRCS(
    doc.go
    protobuf.go
)

GO_TEST_SRCS(protobuf_test.go)

END()

RECURSE(
    gotest
)
