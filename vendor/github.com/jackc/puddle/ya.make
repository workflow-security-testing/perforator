GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.3.0)

SRCS(
    doc.go
    nanotime_unsafe.go
    pool.go
)

GO_TEST_SRCS(internal_test.go)

GO_XTEST_SRCS(pool_test.go)

END()

RECURSE(
    gotest
)
