GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.8.4)

SRCS(
    float16.go
)

GO_XTEST_SRCS(
    float16_bench_test.go
    float16_test.go
)

END()

RECURSE(
    gotest
)
