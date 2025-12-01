GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.31.6)

SRCS(
    json.go
    meta.go
)

GO_TEST_SRCS(
    json_limit_test.go
    meta_test.go
)

GO_XTEST_SRCS(json_test.go)

END()

RECURSE(
    gotest
)
