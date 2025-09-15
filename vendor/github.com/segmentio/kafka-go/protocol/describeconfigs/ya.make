GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    describeconfigs.go
)

GO_TEST_SRCS(describeconfigs_test.go)

END()

RECURSE(
    gotest
)
