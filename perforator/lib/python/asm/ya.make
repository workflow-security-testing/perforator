LIBRARY()

IF(ARCH_X86_64)

PEERDIR(
    perforator/lib/python/asm/x86
)

ELSEIF(ARCH_AARCH64)

PEERDIR(
    perforator/lib/python/asm/arm
)

ENDIF()

END()

RECURSE(
    arm
    x86
)
