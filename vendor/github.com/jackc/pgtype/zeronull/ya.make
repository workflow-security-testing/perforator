GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.12.0)

SRCS(
    doc.go
    float8.go
    int2.go
    int4.go
    int8.go
    text.go
    timestamp.go
    timestamptz.go
    uuid.go
)

GO_XTEST_SRCS(
    float8_test.go
    int2_test.go
    int4_test.go
    int8_test.go
    text_test.go
    timestamp_test.go
    timestamptz_test.go
    uuid_test.go
)

END()

RECURSE(
    # gotest
)
