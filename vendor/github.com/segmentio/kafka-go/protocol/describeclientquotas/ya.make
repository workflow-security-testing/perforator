GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    describeclientquotas.go
)

GO_XTEST_SRCS(describeclientquotas_test.go)

END()

RECURSE(
    gotest
)
