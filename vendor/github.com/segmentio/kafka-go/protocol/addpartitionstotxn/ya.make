GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    addpartitionstotxn.go
)

GO_XTEST_SRCS(addpartitionstotxn_test.go)

END()

RECURSE(
    gotest
)
