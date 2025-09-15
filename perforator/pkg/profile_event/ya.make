GO_LIBRARY()

SRCS(
    async_publisher.go
    model.go
)

GO_TEST_SRCS(async_publisher_test.go)

END()

RECURSE(
    gotest
)
