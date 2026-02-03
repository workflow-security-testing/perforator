LIBRARY()

SRCS(
    render.cpp
)

PEERDIR(
    perforator/lib/profile
    perforator/lib/profile/trie
    perforator/proto/profile

    contrib/libs/rapidjson
)

END()
