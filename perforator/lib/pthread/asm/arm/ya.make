LIBRARY()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/AArch64
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object

    perforator/lib/elf
)

SRCS(
    decode.cpp
)

END()
