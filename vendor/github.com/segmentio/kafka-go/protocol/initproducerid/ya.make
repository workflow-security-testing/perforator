GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    initproducerid.go
)

GO_XTEST_SRCS(initproducerid_test.go)

END()

RECURSE(
    gotest
)
