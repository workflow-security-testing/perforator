GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    describeuserscramcredentials.go
)

GO_XTEST_SRCS(describeuserscramcredentials_test.go)

END()

RECURSE(
    gotest
)
