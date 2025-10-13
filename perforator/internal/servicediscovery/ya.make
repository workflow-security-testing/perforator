GO_LIBRARY()

SRCS(
    config.go
    discoverer.go
    newdiscoverer.go
    predefined.go
)

IF (NOT OPENSOURCE)
    SRCS(
        yp.go
    )
ENDIF()

END()
