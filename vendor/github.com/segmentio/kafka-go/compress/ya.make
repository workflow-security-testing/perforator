GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    compress.go
)

GO_XTEST_SRCS(
    # compress_test.go
)

END()

RECURSE(
    gotest
    gzip
    lz4
    snappy
    zstd
)
