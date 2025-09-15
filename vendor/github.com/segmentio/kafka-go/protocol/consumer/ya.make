GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    consumer.go
)

GO_XTEST_SRCS(consumer_test.go)

END()

RECURSE(
    gotest
)
