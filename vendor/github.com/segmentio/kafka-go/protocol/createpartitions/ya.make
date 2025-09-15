GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    createpartitions.go
)

GO_XTEST_SRCS(createpartitions_test.go)

END()

RECURSE(
    gotest
)
