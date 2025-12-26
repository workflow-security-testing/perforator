IF (NOT OPENSOURCE)
    RECURSE(
        gen_images
        check_new_versions
    )
ENDIF()

RECURSE(
    extract_offsets
    load_offsets
)
