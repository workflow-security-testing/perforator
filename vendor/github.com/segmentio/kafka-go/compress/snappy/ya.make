GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    snappy.go
    xerial.go
)

GO_TEST_SRCS(xerial_test.go)

END()

RECURSE(
    go-xerial-snappy
    gotest
)
