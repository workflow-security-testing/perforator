GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.37.0)

SRCS(
    gen.go
    partialsuccess.go
)

GO_TEST_SRCS(partialsuccess_test.go)

END()

RECURSE(
    envconfig
    gotest
    otlpconfig
    otlptracetest
    retry
)
