LIBRARY()

IF(ARCH_X86_64)

PEERDIR(
    perforator/lib/asm/x86
)

ELSEIF(ARCH_AARCH64)

PEERDIR(
    perforator/lib/asm/arm
)

ENDIF()

END()

RECURSE(
    arm
    x86
)
