GO_LIBRARY()

LICENSE(MIT)

VERSION(v4.18.3)

SRCS(
    batch_results.go
    conn.go
    doc.go
    pool.go
    rows.go
    stat.go
    tx.go
)

GO_XTEST_SRCS(
    # bench_test.go
    # common_test.go
    # conn_test.go
    # pool_test.go
    # tx_test.go
)

END()

RECURSE(
    gotest
)
