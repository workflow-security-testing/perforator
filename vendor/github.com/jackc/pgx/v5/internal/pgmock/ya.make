GO_LIBRARY()

LICENSE(MIT)

VERSION(v5.7.6)

SRCS(
    pgmock.go
)

GO_XTEST_SRCS(pgmock_test.go)

END()

RECURSE(
    gotest
)
