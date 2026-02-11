GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.32.0)

SRCS(
    base.go
    bytes.go
    string.go
    time.go
    types.go
)

GO_TEST_SRCS(
    bytes_test.go
    string_test.go
    types_test.go
)

END()

RECURSE(
    gotest
)
