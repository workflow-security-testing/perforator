GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    rawproduce.go
)

GO_XTEST_SRCS(rawproduce_test.go)

END()

RECURSE(
    gotest
)
