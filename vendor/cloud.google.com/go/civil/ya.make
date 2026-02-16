GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.121.6)

SRCS(
    civil.go
)

GO_TEST_SRCS(civil_test.go)

END()

RECURSE(
    gotest
)
