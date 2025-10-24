GO_PROGRAM()

LICENSE(BSD-3-Clause)

VERSION(v0.7.1)

SRCS(
    main.go
    replace.go
)

GO_TEST_SRCS(replace_test.go)

END()

RECURSE(
    gotest
)
