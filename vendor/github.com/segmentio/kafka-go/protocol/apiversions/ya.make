GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    apiversions.go
)

GO_XTEST_SRCS(apiversions_test.go)

END()

RECURSE(
    gotest
)
