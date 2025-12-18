PROGRAM(pythonparse)

INCLUDE(${ARCADIA_ROOT}/perforator/lib/arch.ya.make.inc)

SRCS(main.cpp)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object

    perforator/lib/python
    perforator/lib/llvmex
)

END()
