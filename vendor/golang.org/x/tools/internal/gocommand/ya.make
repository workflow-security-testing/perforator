GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.39.1-0.20251205192105-907593008619)

SRCS(
    invoke.go
    vendor.go
    version.go
)

IF (OS_LINUX)
    SRCS(
        invoke_unix.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        invoke_unix.go
    )
ENDIF()

IF (OS_WINDOWS)
    SRCS(
        invoke_notunix.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        invoke_unix.go
    )
ENDIF()

END()
