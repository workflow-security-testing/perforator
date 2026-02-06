GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.31.0)

SRCS(
    euckr.go
    tables.go
)

GO_TEST_SRCS(all_test.go)

END()

RECURSE(
    gotest
)
