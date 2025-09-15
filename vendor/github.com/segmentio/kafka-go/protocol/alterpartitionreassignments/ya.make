GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    alterpartitionreassignments.go
)

GO_XTEST_SRCS(alterpartitionreassignments_test.go)

END()

RECURSE(
    gotest
)
