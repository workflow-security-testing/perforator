GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    client_server.go
    http_client.go
    http_server.go
    rpc_client.go
    rpc_server.go
    server_handler.go
)

GO_XTEST_SRCS(
    client_server_test.go
    parallel_support_suite_test.go
)

END()

RECURSE(
    gotest
)
