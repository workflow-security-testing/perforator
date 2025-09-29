GO_LIBRARY()

SRCS(
    async_publisher.go
)

GO_TEST_SRCS(async_publisher_test.go)

END()

RECURSE(
    gotest
)
