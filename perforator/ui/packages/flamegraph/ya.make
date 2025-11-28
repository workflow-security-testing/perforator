TS_TSC()
TS_FILES_GLOB(lib/components/**/*.css)
RUN_JAVASCRIPT_AFTER_BUILD(scripts/copy-through.mjs)

USE_LEGACY_PNPM_VIRTUAL_STORE()

END()

RECURSE_FOR_TESTS(
tests
)
