GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.36.0)

SRCS(
    classify_call.go
    element.go
    errorcode.go
    errorcode_string.go
    qualifier.go
    recv.go
    toonew.go
    types.go
    varkind.go
    zerovalue.go
)

END()

RECURSE(
    typeindex
)
