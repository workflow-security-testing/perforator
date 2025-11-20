DLL()

# Uses GPL-2 code (see LICENSE_RESTRICTION_EXCEPTIONS)
LICENSE(GPL-2.0)

LICENSE_RESTRICTION_EXCEPTIONS(
    perforator/internal/linguist/jvm/analysis
)

PEERDIR(
    contrib/libs/jdk
    contrib/libs/llvm18/lib/Target

    perforator/lib/elf

    perforator/internal/linguist/jvm/unwind/lib
)

SRCS(
    jni.cpp
)

END()
