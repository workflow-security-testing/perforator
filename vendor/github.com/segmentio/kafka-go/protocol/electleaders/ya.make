GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    electleaders.go
)

GO_XTEST_SRCS(electleaders_test.go)

END()

RECURSE(
    gotest
)
