GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    listoffsets.go
)

GO_XTEST_SRCS(listoffsets_test.go)

END()

RECURSE(
    gotest
)
