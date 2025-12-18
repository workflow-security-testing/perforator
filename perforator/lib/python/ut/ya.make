GTEST()

INCLUDE(${ARCADIA_ROOT}/perforator/lib/arch.ya.make.inc)

PEERDIR(
    contrib/libs/re2

    library/cpp/logger/global

    perforator/lib/python
)

SRCS(
    parse_version_ut.cpp
)

END()
