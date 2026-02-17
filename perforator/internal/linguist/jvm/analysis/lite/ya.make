LIBRARY()

SRCS(
    lite_analysis.cpp
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Target/X86
    
    perforator/lib/llvmex

    perforator/internal/linguist/jvm/analysis/api
)

END()
