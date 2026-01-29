GO_LIBRARY()

SRCS(
    cli.go
)

IF (OPENSOURCE)
    SRCS(
        token.go
    )
ELSE()
    SRCS(
        token_yandex.go
    )
ENDIF()

END()
