IF (NOT OPENSOURCE)
    RECURSE(
        alerts
        docs
        opensource
        recipes
        release
        sandbox
        scripts
        tasklets
        ops
        v0
    )
ENDIF()

IF (NOT CI)
    RECURSE(ui)
ENDIF()

RECURSE(
    agent
    bundle
    cmd
    ebpf
    internal
    lib
    pkg
    proto
    symbolizer
    tests
    tools
    util
)
