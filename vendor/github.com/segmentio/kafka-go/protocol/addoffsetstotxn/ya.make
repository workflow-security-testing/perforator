GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    addoffsetstotxn.go
)

GO_XTEST_SRCS(addoffsetstotxn_test.go)

END()

RECURSE(
    gotest
)
