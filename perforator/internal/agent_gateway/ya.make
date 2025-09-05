GO_LIBRARY()

PEERDIR(
    perforator/proto/storage
    perforator/pkg/storage/profile/compound
)

SRCS(
    config.go
    custom_profiling_operation.go
    opts.go
    sampler.go
    server.go
    storage.go
)

END()
