LIBRARY()

# This library is backport of SFrame support in llvm-22+

LICENSE(
    Apache-2.0 WITH LLVM-exception
)

SRCS(
    SFrameParser.cpp
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/DebugInfo/DWARF
    contrib/libs/llvm18/lib/DebugInfo/Symbolize
    contrib/libs/llvm18/lib/Target
    contrib/libs/llvm18/lib/Target/X86
)

END()
