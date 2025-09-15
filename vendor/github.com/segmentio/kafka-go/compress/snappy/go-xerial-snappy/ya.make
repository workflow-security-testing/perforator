GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    snappy.go
)

GO_TEST_SRCS(snappy_test.go)

END()

RECURSE(
    gotest
)
