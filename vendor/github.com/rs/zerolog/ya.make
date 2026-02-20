GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.34.0)

SRCS(
    array.go
    console.go
    context.go
    ctx.go
    encoder.go
    encoder_json.go
    event.go
    fields.go
    globals.go
    go112.go
    hook.go
    log.go
    sampler.go
    writer.go
)

GO_TEST_SRCS(
    array_test.go
    benchmark_test.go
    ctx_test.go
    event_test.go
    hook_test.go
    log_test.go
    sampler_test.go
)

GO_XTEST_SRCS(
    console_test.go
    log_example_test.go
)

IF (OS_LINUX)
    SRCS(
        syslog.go
    )

    GO_TEST_SRCS(
        syslog_test.go
        writer_test.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        syslog.go
    )

    GO_TEST_SRCS(
        syslog_test.go
        writer_test.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        syslog.go
    )

    GO_TEST_SRCS(
        syslog_test.go
        writer_test.go
    )
ENDIF()

END()

RECURSE(
    gotest
    internal
    log
)
