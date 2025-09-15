GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    offsetcommit.go
)

GO_XTEST_SRCS(offsetcommit_test.go)

END()

RECURSE(
    gotest
)
