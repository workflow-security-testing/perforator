GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    endtxn.go
)

GO_XTEST_SRCS(endtxn_test.go)

END()

RECURSE(
    gotest
)
