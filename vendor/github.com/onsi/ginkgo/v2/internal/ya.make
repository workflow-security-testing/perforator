GO_LIBRARY()

LICENSE(MIT)

VERSION(v2.22.2)

SRCS(
    counter.go
    failer.go
    focus.go
    group.go
    node.go
    ordering.go
    output_interceptor.go
    progress_report.go
    progress_reporter_manager.go
    report_entry.go
    spec.go
    spec_context.go
    suite.go
    tree.go
    writer.go
)

GO_XTEST_SRCS(
    counter_test.go
    failer_test.go
    focus_test.go
    internal_suite_test.go
    node_test.go
    ordering_test.go
    output_interceptor_test.go
    progress_report_test.go
    progress_reporter_manager_test.go
    report_entry_test.go
    spec_context_test.go
    spec_test.go
    suite_test.go
    tree_test.go
    writer_test.go
)

IF (OS_LINUX)
    SRCS(
        output_interceptor_unix.go
        progress_report_unix.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        output_interceptor_unix.go
        progress_report_bsd.go
    )
ENDIF()

IF (OS_WINDOWS)
    SRCS(
        output_interceptor_win.go
        progress_report_win.go
    )
ENDIF()

IF (OS_ANDROID)
    SRCS(
        output_interceptor_unix.go
        progress_report_unix.go
    )
ENDIF()

END()

RECURSE(
    global
    # gotest
    internal_integration
    interrupt_handler
    parallel_support
    test_helpers
    testingtproxy
)
