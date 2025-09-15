GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    fetch.go
)

GO_XTEST_SRCS(fetch_test.go)

END()

RECURSE(
    gotest
)
