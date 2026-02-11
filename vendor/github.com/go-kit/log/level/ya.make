GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.2.1)

SRCS(
    doc.go
    level.go
)

GO_XTEST_SRCS(
    benchmark_test.go
    example_test.go
    level_test.go
)

END()

RECURSE(
    gotest
)
