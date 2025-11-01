LIBRARY()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/ARM
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

IF (ARCH_x86_64)
    PEERDIR(perforator/lib/python/asm/x86)
ELSEIF (ARCH_AARCH64)
    PEERDIR(perforator/lib/python/asm/arm)
ENDIF()

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object
    contrib/libs/re2

    perforator/lib/elf
    perforator/lib/tls/parser
    perforator/lib/llvmex
    perforator/lib/python/asm
)

SRCS(
    python.cpp
)

END()

RECURSE(
    asm
    cli
)

RECURSE_FOR_TESTS(
    ut
)
