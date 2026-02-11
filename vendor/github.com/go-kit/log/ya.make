GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.2.1)

SRCS(
    doc.go
    json_logger.go
    log.go
    logfmt_logger.go
    nop_logger.go
    stdlib.go
    sync.go
    value.go
)

GO_TEST_SRCS(stdlib_test.go)

GO_XTEST_SRCS(
    benchmark_test.go
    concurrency_test.go
    example_test.go
    json_logger_test.go
    log_test.go
    logfmt_logger_test.go
    nop_logger_test.go
    sync_test.go
    value_test.go
)

END()

RECURSE(
    gotest
    level
)
