GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    formatter.go
)

GO_XTEST_SRCS(
    formatter_suite_test.go
    formatter_test.go
)

IF (OS_LINUX)
    SRCS(
        colorable_others.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        colorable_others.go
    )
ENDIF()

IF (OS_WINDOWS)
    SRCS(
        colorable_windows.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        colorable_others.go
    )
ENDIF()

IF (OS_EMSCRIPTEN)
    SRCS(
        colorable_others.go
    )
ENDIF()

END()

RECURSE(
    gotest
)
