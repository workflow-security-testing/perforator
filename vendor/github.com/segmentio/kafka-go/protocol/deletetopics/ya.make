GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    deletetopics.go
)

GO_XTEST_SRCS(deletetopics_test.go)

END()

RECURSE(
    gotest
)
