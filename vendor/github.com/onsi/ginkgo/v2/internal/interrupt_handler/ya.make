GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    interrupt_handler.go
)

GO_XTEST_SRCS(interrupthandler_suite_test.go)

IF (OS_LINUX)
    SRCS(
        sigquit_swallower_unix.go
    )

    GO_XTEST_SRCS(interrupt_handler_test.go)
ENDIF()

IF (OS_DARWIN)
    SRCS(
        sigquit_swallower_unix.go
    )

    GO_XTEST_SRCS(interrupt_handler_test.go)
ENDIF()

IF (OS_WINDOWS)
    SRCS(
        sigquit_swallower_windows.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        sigquit_swallower_unix.go
    )

    GO_XTEST_SRCS(interrupt_handler_test.go)
ENDIF()

END()

RECURSE(
    gotest
)
