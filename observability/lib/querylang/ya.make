GO_LIBRARY()

SRCS(
    selector_ast.go
    selector_iter.go
    selector_repr.go
    expression_ast.go
    helpers.go
    parser.go
    tools.go
)

GO_XTEST_SRCS(
    selector_iter_test.go
)

END()

RECURSE(
    gotest
    operator
    parser
    template
)
