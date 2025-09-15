GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    deleteacls.go
)

GO_XTEST_SRCS(deleteacls_test.go)

END()

RECURSE(
    gotest
)
