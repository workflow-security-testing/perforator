GO_LIBRARY()

LICENSE(MIT)

VERSION(v5.7.6)

SRCS(
    bgreader.go
)

GO_XTEST_SRCS(bgreader_test.go)

END()

RECURSE(
    gotest
)
