GO_LIBRARY()

SRCS(
    cgroupevent.go
    event_listener.go
    pidprofile.go
    pods_cgroup_tracker.go
    profile_builder.go
    profiler.go
    sample_consumer.go
    sample_consumer_registry.go
    sample_filter.go
    stack_processor.go
    uprobe_registry.go
)

GO_TEST_SRCS(sample_consumer_test.go)

END()

RECURSE_FOR_TESTS(
    gotest
)
