# yo ignore:file
RECURSE(
    dummies
    perfbuf_bench
    python
    sample_reader_bench
)

IF (NOT OPENSOURCE)
    RECURSE(
        yandex-specific
    )
ENDIF()
