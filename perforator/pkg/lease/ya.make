GO_LIBRARY()

SRCS(
    models.go
    lease.go
    test_suite.go
)

END()

RECURSE(
    postgres
)
