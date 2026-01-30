PY3TEST()

TEST_SRCS(
    __init__.py
    test_globs.py
    test_utils_copy_files_with_exclusions.py
)

PEERDIR(
    devtools/frontend_build_platform/nots/builder/api
)

END()
