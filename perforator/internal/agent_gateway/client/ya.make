GO_LIBRARY()

SRCS(
    client.go
    config.go
)

END()

RECURSE(
    custom_profiling_operation
    storage
)
