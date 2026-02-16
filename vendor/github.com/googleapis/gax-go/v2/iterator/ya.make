GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v2.15.0)

SRCS(
    iterator.go
)

GO_XTEST_SRCS(example_test.go)

END()

RECURSE(
    gotest
)
