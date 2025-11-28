GTEST()

REQUIREMENTS(ram:16)

SIZE(MEDIUM)

SRCS(
    merge_ut.cpp
)

PEERDIR(
    perforator/lib/profile/ut/lib
)

DEPENDS(
    perforator/lib/profile/ut/yandex-specific/testprofiles/merge_yabs
)

END()
