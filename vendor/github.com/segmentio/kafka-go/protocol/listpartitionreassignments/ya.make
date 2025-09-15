GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    listpartitionreassignments.go
)

GO_XTEST_SRCS(listpartitionreassignments_test.go)

END()

RECURSE(
    gotest
)
