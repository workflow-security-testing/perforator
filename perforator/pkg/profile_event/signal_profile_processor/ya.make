GO_LIBRARY()

SRCS(
    config.go
    proxy_processor.go
    service.go
)

GO_TEST_SRCS(service_test.go)

END()

RECURSE(
    gotest
)
