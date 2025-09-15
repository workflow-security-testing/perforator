GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    joingroup.go
)

GO_XTEST_SRCS(joingroup_test.go)

END()

RECURSE(
    gotest
)
