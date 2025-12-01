GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    code_location.go
    config.go
    deprecated_types.go
    deprecation_support.go
    enum_support.go
    errors.go
    file_filter.go
    flags.go
    label_filter.go
    report_entry.go
    types.go
    version.go
)

GO_XTEST_SRCS(
    code_location_test.go
    config_test.go
    deprecated_support_test.go
    deprecated_types_test.go
    errors_test.go
    file_filters_test.go
    flags_test.go
    label_filter_test.go
    types_suite_test.go
    types_test.go
)

END()

RECURSE(
    gotest
)
