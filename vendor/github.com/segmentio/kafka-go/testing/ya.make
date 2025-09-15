GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    conn.go
    version.go
)

GO_TEST_SRCS(version_test.go)

END()

RECURSE(
    gotest
)
