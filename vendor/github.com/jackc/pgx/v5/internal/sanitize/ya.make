GO_LIBRARY()

LICENSE(MIT)

VERSION(v5.7.6)

SRCS(
    sanitize.go
)

GO_XTEST_SRCS(
    sanitize_bench_test.go
    sanitize_fuzz_test.go
    sanitize_test.go
)

END()

RECURSE(
    gotest
)
