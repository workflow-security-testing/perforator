GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    metadata.go
)

GO_XTEST_SRCS(metadata_test.go)

END()

RECURSE(
    gotest
)
