GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    alterclientquotas.go
)

GO_XTEST_SRCS(alterclientquotas_test.go)

END()

RECURSE(
    gotest
)
