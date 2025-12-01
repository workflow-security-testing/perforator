GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.0.0-20241104100929-3ea5e8cea738)

SRCS(
    ptr.go
)

GO_XTEST_SRCS(ptr_test.go)

END()

RECURSE(
    gotest
)
