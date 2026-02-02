LIBRARY()

SRCS(
    trie.cpp
)

PEERDIR(
    library/cpp/containers/absl_flat_hash
)

END()

RECURSE_FOR_TESTS(ut)
