GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.6.0)

SRCS(
    decode.go
    doc.go
    encode.go
    jsonstring.go
)

GO_TEST_SRCS(
    decode-bench_test.go
    decode_test.go
    encode_internal_test.go
)

GO_XTEST_SRCS(
    encode_test.go
    example_test.go
)

END()

RECURSE(
    gotest
)
