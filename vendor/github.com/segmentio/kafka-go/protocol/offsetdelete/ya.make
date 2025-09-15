GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    offsetdelete.go
)

GO_XTEST_SRCS(offsetdelete_test.go)

END()

RECURSE(
    gotest
)
