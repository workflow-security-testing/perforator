TS_VITE()

SRCS(
    index.html
)

TS_TYPECHECK()

TS_ESLINT_CONFIG(.eslintrc.js)

TS_CONFIG(tsconfig.json)

TS_STYLELINT(.stylelintrc)

USE_LEGACY_PNPM_VIRTUAL_STORE()

END()

RECURSE_FOR_TESTS(
  tests
)
