GO_LIBRARY()

GO_TEST_SRCS(
    basic_integration_test.go
    cpo_integration_test.go
)

SRCS(
    test_env.go
    suite.go
)

END()
