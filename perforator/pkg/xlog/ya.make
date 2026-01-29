GO_LIBRARY()

SRCS(
    bound.go
    init.go
    xlog.go
)

END()

RECURSE(
    logmetrics
)
