GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.16.3)

SRCS(
    auth.go
    threelegged.go
)

GO_TEST_SRCS(
    auth_test.go
    auth_token_async_test.go
    threelegged_test.go
)

GO_XTEST_SRCS(
    # example_test.go
)

END()

RECURSE(
    credentials
    gotest
    grpctransport
    httptransport
    internal
)
