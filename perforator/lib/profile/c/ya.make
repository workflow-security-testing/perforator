LIBRARY()

SRCS(
    error.cpp
    flamegraph.cpp
    merge.cpp
    profile.cpp
    string.cpp
)

PEERDIR(
    perforator/lib/profile
    perforator/lib/profile/flamegraph
    perforator/proto/pprofprofile
    perforator/proto/profile
)

END()
