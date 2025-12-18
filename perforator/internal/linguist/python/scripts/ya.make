IF (NOT OPENSOURCE)
    RECURSE(
        gen_images
    )
ENDIF()

RECURSE(
    extract_offsets
    load_offsets
)
