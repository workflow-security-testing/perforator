GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    alterconfigs.go
)

GO_XTEST_SRCS(alterconfigs_test.go)

END()

RECURSE(
    gotest
)
