GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    default_reporter.go
    deprecated_reporter.go
    json_report.go
    junit_report.go
    reporter.go
    teamcity_report.go
)

GO_XTEST_SRCS(
    default_reporter_test.go
    deprecated_reporter_test.go
    json_report_test.go
    junit_report_test.go
    reporters_suite_test.go
    teamcity_report_test.go
)

END()

RECURSE(
    gotest
)
