GTEST()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/ARM
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object
    contrib/libs/llvm18/lib/Support
    contrib/libs/llvm18/lib/Target
    contrib/libs/llvm18/lib/Target/ARM
    contrib/libs/llvm18/lib/Target/ARM/Disassembler
    contrib/libs/llvm18/lib/Target/ARM/MCTargetDesc

    library/cpp/logger/global
    library/cpp/testing/gtest
    library/cpp/testing/gtest

    perforator/lib/asm/arm
)

SRCS(
    evaluator_ut.cpp
)

END()
