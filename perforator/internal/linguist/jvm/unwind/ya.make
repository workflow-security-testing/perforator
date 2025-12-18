RECURSE(
    cheatsheets/tool
    configure
    lib
)

IF (ARCH_X86_64)

RECURSE(
    jni
    sample
)

ENDIF()
