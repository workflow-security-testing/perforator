GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    incrementalalterconfigs.go
)

GO_XTEST_SRCS(incrementalalterconfigs_test.go)

END()

RECURSE(
    gotest
)
