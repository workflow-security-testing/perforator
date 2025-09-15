GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    describeacls.go
)

GO_XTEST_SRCS(describeacls_test.go)

END()

RECURSE(
    gotest
)
