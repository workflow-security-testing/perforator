GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    sasl.go
)

GO_XTEST_SRCS(
    # sasl_test.go
)

END()

RECURSE(
    gotest
    plain
    scram
)
