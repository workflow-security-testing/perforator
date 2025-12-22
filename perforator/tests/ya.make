RECURSE(integration)

IF (NOT OPENSOURCE) 
    RECURSE(
        yandex-specific
    )
ENDIF()
