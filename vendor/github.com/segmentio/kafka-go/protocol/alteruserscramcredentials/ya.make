GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    alteruserscramcredentials.go
)

GO_XTEST_SRCS(alteruserscramcredentials_test.go)

END()

RECURSE(
    gotest
)
