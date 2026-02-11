GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.12.0)

SRCS(
    decimal.go
)

GO_XTEST_SRCS(
    # decimal_test.go
)

END()

RECURSE(
    gotest
)
