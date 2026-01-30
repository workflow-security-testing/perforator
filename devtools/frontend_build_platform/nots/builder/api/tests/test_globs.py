"""Tests for glob pattern matching utilities."""

import pytest

from devtools.frontend_build_platform.nots.builder.api.globs import GlobMatcher


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match files in current directory only
        ('file.ts', True),
        ('package.json', True),
        # Should NOT match files in subdirectories
        ('src/file.ts', False),
        ('src/nested/file.ts', False),
    ],
)
def test_matches_all_files_in_curdir(path, expected):
    """Test pattern: * - all files in curdir"""
    matcher = GlobMatcher(['*'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match files directly in src/
        ('src/file.ts', True),
        ('src/index.js', True),
        # Should NOT match files in subdirectories of src/
        ('src/nested/file.ts', False),
        ('src/deep/nested/file.ts', False),
        # Should NOT match files in other directories
        ('other/file.ts', False),
        # Should NOT match files in curdir
        ('file.ts', False),
    ],
)
def test_matches_all_files_in_directory_non_recursive(path, expected):
    """Test pattern: src/* - all files in src directory, not recursive"""
    matcher = GlobMatcher(['src/*'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match files directly in src/
        ('src/file.ts', True),
        # Should match files in subdirectories
        ('src/nested/file.ts', True),
        ('src/deep/nested/file.ts', True),
        # Should NOT match files in other directories
        ('other/file.ts', False),
        ('build/output.js', False),
    ],
)
def test_matches_all_files_in_directory_recursive(path, expected):
    """Test pattern: src/**/* - all files in src directory, recursive"""
    matcher = GlobMatcher(['src/**/*'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match .md files in docs/
        ('docs/readme.md', True),
        ('docs/nested/guide.md', True),
        # Should match .md files in spec/
        ('spec/api.md', True),
        ('spec/nested/details.md', True),
        # Should NOT match non-.md files
        ('docs/file.ts', False),
        ('spec/file.js', False),
        # Should NOT match .md files in other directories
        ('src/readme.md', False),
        ('readme.md', False),
    ],
)
def test_matches_with_alternation_and_extension(path, expected):
    """Test pattern: (docs|spec)/**/*.md - all markdown files in docs and spec directories"""
    matcher = GlobMatcher(['(docs|spec)/**/*.md'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match spec.js files
        ('src/file.spec.js', True),
        ('src/nested/file.spec.js', True),
        # Should match spec.ts files
        ('src/file.spec.ts', True),
        ('src/nested/file.spec.ts', True),
        # Should match test.js files
        ('src/file.test.js', True),
        ('src/nested/file.test.js', True),
        # Should match test.ts files
        ('src/file.test.ts', True),
        ('src/nested/file.test.ts', True),
        # Should NOT match regular files
        ('src/file.js', False),
        ('src/file.ts', False),
        # Should NOT match files in other directories
        ('test/file.spec.js', False),
    ],
)
def test_matches_complex_alternation_pattern(path, expected):
    """Test pattern: src/**/*.(spec|test).(js|ts) - test files in src directory"""
    matcher = GlobMatcher(['src/**/*.(spec|test).(js|ts)'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match files starting with 'a' in curdir
        ('app.ts', True),
        ('api.js', True),
        ('a', True),
        # Should NOT match files not starting with 'a'
        ('file.ts', False),
        ('bapp.ts', False),
        # Should NOT match files in subdirectories
        ('src/app.ts', False),
    ],
)
def test_matches_prefix_pattern(path, expected):
    """Test pattern: a* - all files starting with 'a' in curdir"""
    matcher = GlobMatcher(['a*'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, expected',
    [
        # Should match *.test.ts in curdir
        ('file.test.ts', True),
        # Should match files in build/
        ('build/output.js', True),
        ('build/nested/output.js', True),
        # Should match files in node_modules/
        ('node_modules/package.json', True),
        # Should NOT match regular files
        ('src/file.ts', False),
    ],
)
def test_matches_multiple_patterns(path, expected):
    """Test matching against multiple patterns"""
    matcher = GlobMatcher(['*.test.ts', 'build/**/*', 'node_modules/**/*'])
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'path, patterns, expected',
    [
        # Empty patterns list
        ('file.ts', [], False),
        # Empty path
        ('', ['*'], False),
        # Pattern with no wildcards should match exactly
        ('exact.txt', ['exact.txt'], True),
        ('other.txt', ['exact.txt'], False),
    ],
)
def test_matches_edge_cases(path, patterns, expected):
    """Test edge cases in glob matching"""
    matcher = GlobMatcher(patterns)
    assert matcher.matches(path) == expected


@pytest.mark.parametrize(
    'dir_path, expected',
    [
        # Should match directories with patterns ending in **/*
        ('build', True),
        ('build/', True),
        ('node_modules', True),
        # Should match directories starting with 'a' (a*/**/* pattern)
        ('assets', True),
        ('assets/img', True),
        ('app', True),
        # Should NOT match other directories
        ('src', False),
        ('other', False),
        ('test', False),
    ],
)
def test_glob_matcher_whole_dir(dir_path, expected):
    """Test GlobMatcher.matches_whole_dir() method"""
    matcher = GlobMatcher(['build/**/*', 'node_modules/**/*', 'a*/**/*'])
    assert matcher.matches_whole_dir(dir_path) == expected
