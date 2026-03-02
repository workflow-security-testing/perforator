GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v2.0.0-20200109203555-b30bc20e4fd1)

SRCS(
    doc.go
    format.go
    handler.go
    handler_go14.go
    logger.go
    root.go
)

GO_TEST_SRCS(
    bench_test.go
    log15_test.go
    logger_test.go
)

IF (OS_LINUX)
    SRCS(
        syslog.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        syslog.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        syslog.go
    )
ENDIF()

IF (OS_EMSCRIPTEN)
    SRCS(
        syslog.go
    )
ENDIF()

END()

RECURSE(
    # ext
    gotest
)
