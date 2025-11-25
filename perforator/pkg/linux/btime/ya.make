GO_LIBRARY()

SRCS(
    btime.go
)

GO_TEST_SRCS(
    btime_test.go
)

END()

RECURSE(
    gotest
)
