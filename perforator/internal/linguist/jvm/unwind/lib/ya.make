LIBRARY()

INCLUDE(gen.inc)
IF (DEFINED JDK_NOT_CONFIGURED)
    SRCS(
        fallback.cpp
    )
ELSE()

    SRCS(
        cheatsheet.cpp
        offsets.cpp
    )

ENDIF()

PEERDIR(
    perforator/internal/linguist/jvm/analysis
)

# TODO: get rid of this
NO_COMPILER_WARNINGS()


END()
