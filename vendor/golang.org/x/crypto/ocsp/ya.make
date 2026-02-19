GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.46.0)

SRCS(
    ocsp.go
)

GO_TEST_SRCS(ocsp_test.go)

END()

RECURSE(
    gotest
)
