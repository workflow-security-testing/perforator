GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.39.1-0.20251205192105-907593008619)

SRCS(
    id.go
    labels.go
    log.go
    printer.go
    trace.go
)

END()

RECURSE(
    eventtest
    metric
    prometheus
)
