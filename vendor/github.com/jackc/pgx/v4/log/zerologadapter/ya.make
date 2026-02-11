GO_LIBRARY()

LICENSE(MIT)

VERSION(v4.18.3)

SRCS(
    adapter.go
)

GO_XTEST_SRCS(adapter_test.go)

END()

RECURSE(
    gotest
)
