LIBRARY()

# According to lawyers, this effectively depends on OpenJDK which is licensed under GPL v2
LICENSE(GPL-2.0)


INCLUDE(gen.inc)
SRCS(
    static_analysis.cpp
)
IF (DEFINED JDK_NOT_CONFIGURED)
    SRCS(
        fallback.cpp
    )
ELSE()
    SRCS(
        offsets.cpp
    )

    # TODO: get rid of this
    NO_COMPILER_WARNINGS()
ENDIF()

PEERDIR(
    perforator/internal/linguist/jvm/analysis/offset_registry
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Target/X86
    
    perforator/lib/llvmex
)

END()
