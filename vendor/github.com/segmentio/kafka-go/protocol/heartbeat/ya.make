GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    heartbeat.go
)

GO_XTEST_SRCS(heartbeat_test.go)

END()

RECURSE(
    gotest
)
