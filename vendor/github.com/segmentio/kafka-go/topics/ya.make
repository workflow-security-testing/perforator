GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    list_topics.go
)

GO_TEST_SRCS(
    # list_topics_test.go
)

END()

RECURSE(
    gotest
)
