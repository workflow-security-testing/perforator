# yo ignore:file
RECURSE(
    dummies
    perfbuf_bench
    python
)

IF (NOT OPENSOURCE)
    RECURSE(
        yandex-specific
    )
ENDIF()
