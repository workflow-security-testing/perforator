LIBRARY()

IF (ARCH_X86_64)

PEERDIR(
    perforator/lib/php/asm/x86
)

ELSEIF(ARCH_AARCH64)

PEERDIR(
    perforator/lib/php/asm/arm
)

ENDIF()

END()
