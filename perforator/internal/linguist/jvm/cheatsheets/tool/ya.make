PROGRAM()

# Uses GPL-2 code (see LICENSE_RESTRICTION_EXCEPTIONS)
LICENSE(GPL-2.0)

LICENSE_RESTRICTION_EXCEPTIONS(
    perforator/internal/linguist/jvm/analysis/static
    perforator/internal/linguist/jvm/analysis/offset_registry
)

SRCS(main.cpp)

PEERDIR(
    perforator/internal/linguist/jvm/analysis/static    
    library/cpp/json
    library/cpp/getopt
)

END()
