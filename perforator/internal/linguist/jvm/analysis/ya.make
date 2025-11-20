LIBRARY()

# According to lawyers, this effectively depends on OpenJDK which is licensed under GPL v2
LICENSE(GPL-2.0)

SRCS(
    analyzer_impl.cpp
    analyzer.cpp
    offset_registry.cpp
    output.cpp
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Target/X86
    
    perforator/lib/llvmex
)

END()
