PROGRAM()

# Uses GPL-2 code (see LICENSE_RESTRICTION_EXCEPTIONS)
LICENSE(GPL-2.0)

LICENSE_RESTRICTION_EXCEPTIONS(
    perforator/internal/linguist/jvm/analysis
)

SRCS(main.cpp)

PEERDIR(
    perforator/internal/linguist/jvm/unwind/lib
    library/cpp/json
)

END()
