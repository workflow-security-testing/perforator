GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.12.0)

SRCS(
    uuid.go
)

GO_XTEST_SRCS(
    # uuid_test.go
)

END()

RECURSE(
    gotest
)
