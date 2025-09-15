GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    createacls.go
)

GO_XTEST_SRCS(createacls_test.go)

END()

RECURSE(
    gotest
)
