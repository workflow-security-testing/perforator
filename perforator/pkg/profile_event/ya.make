GO_LIBRARY()

IF (OPENSOURCE)
    SRCS(
        model.go
    )
ELSE()
    SRCS(
        model_yandex.go
    )
ENDIF()

END()

RECURSE(
    async_publisher
    signal_profile_processor
)
