GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.121.6)

SRCS(
    protostruct.go
)

GO_TEST_SRCS(protostruct_test.go)

END()

RECURSE(
    gotest
)
