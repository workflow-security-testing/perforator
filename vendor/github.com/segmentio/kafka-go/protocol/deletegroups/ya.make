GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    deletegroups.go
)

GO_XTEST_SRCS(deletegroups_test.go)

END()

RECURSE(
    gotest
)
