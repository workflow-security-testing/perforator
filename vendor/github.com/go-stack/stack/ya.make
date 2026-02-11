GO_LIBRARY()

LICENSE(MIT)

VERSION(v1.8.1)

SRCS(
    stack.go
)

GO_XTEST_SRCS(
    format_test.go
    # stack-go19_test.go
    # stack_test.go
)

END()

RECURSE(
    gotest
)
