package versioncfg

import (
	"fmt"
	"os"
	"strings"
)

// ReadSemconvPackage extracts SEMCONV_PKG from versions.mk.
func ReadSemconvPackage(path string) (string, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return "", fmt.Errorf("read %s: %w", path, err)
	}

	vars := make(map[string]string)
	for _, rawLine := range strings.Split(string(data), "\n") {
		line := strings.TrimSpace(rawLine)
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}

		name, value, ok := strings.Cut(line, ":=")
		if !ok {
			continue
		}
		key := strings.TrimSpace(name)
		val := strings.TrimSpace(value)
		if key == "" {
			continue
		}
		vars[key] = val
	}

	pkg, ok := vars["SEMCONV_PKG"]
	if !ok || pkg == "" {
		return "", fmt.Errorf("SEMCONV_PKG not found in %s", path)
	}

	expanded := pkg
	for i := 0; i < 10; i++ {
		changed := false
		for k, v := range vars {
			placeholder := "$(" + k + ")"
			if strings.Contains(expanded, placeholder) {
				expanded = strings.ReplaceAll(expanded, placeholder, v)
				changed = true
			}
		}
		if !changed {
			break
		}
	}

	if strings.Contains(expanded, "$(") {
		return "", fmt.Errorf("failed to expand SEMCONV_PKG=%q from %s", pkg, path)
	}
	if expanded == "" {
		return "", fmt.Errorf("empty SEMCONV_PKG in %s", path)
	}

	return expanded, nil
}
