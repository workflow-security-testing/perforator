"""
Glob pattern matching utilities for file filtering.

Supports:
- Simple wildcards: *.test.ts
- Recursive patterns: build/**/*
- Alternation patterns: (.idea|.vscode)/**/*
"""

import re


class GlobMatcher:
    """
    Matches file paths against glob patterns using regex conversion.

    Based on devtools/ymake/symbols/globs.cpp PatternToRegexp implementation.
    Alternation patterns like (a|b|c) are handled by regex alternation syntax.
    """

    def __init__(self, patterns: list[str]):
        """
        Initialize matcher with list of glob patterns.

        Args:
            patterns: List of glob patterns (may contain alternations)
        """
        self._patterns = patterns
        self._regexes = [self._pattern_to_regex(p) for p in patterns]
        self._dir_regexes = [self._pattern_to_regex(p[:-4], for_dir=True) for p in patterns if p.endswith('**/*')]

    def matches(self, path: str) -> bool:
        """
        Check if path matches any of the patterns.

        Args:
            path: File path to check (relative path with forward slashes)

        Returns:
            True if path matches any pattern
        """
        # Empty path should not match any pattern
        if not path:
            return False

        for regex in self._regexes:
            if regex.match(path):
                return True
        return False

    def matches_whole_dir(self, path: str) -> bool:
        """
        Check if entire directory matches pattern (for patterns ending with **/*).

        Args:
            path: Directory path to check (relative path with forward slashes)

        Returns:
            True if directory matches any pattern ending with **/*
        """
        # Add trailing slash for directory matching
        dir_path = path if path.endswith('/') else path + '/'

        for regex in self._dir_regexes:
            if regex.match(dir_path):
                return True
        return False

    @staticmethod
    def _pattern_to_regex(pattern: str, for_dir=False) -> re.Pattern:
        """
        Convert glob pattern to compiled regex.

        Based on PatternToRegexp from globs.cpp (lines 341-358):
        - * matches any characters except /
        - ? matches any single character except /
        - ** matches any path segments (including /)
        - . is escaped
        - (a|b|c) is converted to regex alternation (a|b|c)

        Args:
            pattern: Glob pattern
            for_dir: If True, pattern is for directory matching

        Returns:
            Compiled regex pattern
        """

        result = ['^']
        segments = pattern.split('/')
        need_sep = False

        for segment in segments:
            if not segment:
                continue

            if need_sep:
                result.append('/')
            need_sep = True

            if segment == '**':
                # ** matches zero or more path segments
                # Pattern: (.*/)? - matches any characters followed by /, or nothing
                result.append('(.*' + '/' + ')?')
                need_sep = False
            else:
                # Convert segment to regex
                result.append(GlobMatcher._glob_segment_to_regex(segment))

        if for_dir:
            result.append('/.*')
        result.append('$')
        return re.compile(''.join(result))

    @staticmethod
    def _glob_segment_to_regex(segment: str) -> str:
        """
        Convert a single glob segment to regex.

        Based on GlobSegmentToRegex from https://a.yandex-team.ru/arcadia/devtools/ymake/symbols/globs.cpp

        Args:
            segment: Glob segment (part between /)

        Returns:
            Regex string for the segment
        """
        result = []
        i = 0
        while i < len(segment):
            ch = segment[i]

            if ch == '*':
                # `*` matches any characters except /
                result.append('[^/]*')
            elif ch == '?':
                # `?` matches any single character except /
                result.append('[^/]')
            elif ch == '.':
                # Escape regex special characters
                result.append('\\.')
            else:
                # Regular character
                result.append(ch)

            i += 1

        return ''.join(result)
