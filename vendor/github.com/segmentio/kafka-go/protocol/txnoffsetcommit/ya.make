GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    txnoffsetcommit.go
)

GO_XTEST_SRCS(txnoffsetcommit_test.go)

END()

RECURSE(
    gotest
)
