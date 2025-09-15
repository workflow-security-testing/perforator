GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    leavegroup.go
)

GO_XTEST_SRCS(leavegroup_test.go)

END()

RECURSE(
    gotest
)
