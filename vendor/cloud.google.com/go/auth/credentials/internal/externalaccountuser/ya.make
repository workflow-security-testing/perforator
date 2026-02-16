GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v0.16.3)

SRCS(
    externalaccountuser.go
)

GO_TEST_SRCS(externalaccountuser_test.go)

END()

RECURSE(
    gotest
)
