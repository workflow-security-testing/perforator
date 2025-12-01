GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    core_dsl.go
    decorator_dsl.go
    deprecated_dsl.go
    ginkgo_t_dsl.go
    reporting_dsl.go
    table_dsl.go
)

END()

RECURSE(
    config
    docs
    dsl
    extensions
    formatter
    ginkgo
    integration
    internal
    reporters
    types
)
