GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.31.6)

SRCS(
    deep_equal.go
)

GO_TEST_SRCS(deep_equal_test.go)

END()

RECURSE(
    gotest
)
