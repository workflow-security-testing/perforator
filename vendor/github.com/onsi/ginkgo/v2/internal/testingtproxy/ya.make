GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    testing_t_proxy.go
)

GO_XTEST_SRCS(
    testingtproxy_suite_test.go
    testingtproxy_test.go
)

END()

RECURSE(
    gotest
)
