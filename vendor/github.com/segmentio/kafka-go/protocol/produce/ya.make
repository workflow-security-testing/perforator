GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    produce.go
)

GO_XTEST_SRCS(produce_test.go)

END()

RECURSE(
    gotest
)
