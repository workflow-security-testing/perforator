PROGRAM(preprocessing)

SRCS(
    main.cpp
)

PEERDIR(
    library/cpp/getopt
    library/cpp/streams/zstd
    perforator/agent/preprocessing/lib
    perforator/agent/preprocessing/proto/parse
    perforator/agent/preprocessing/proto/tls
    perforator/agent/preprocessing/proto/unwind
    perforator/lib/llvmex
)

END()
