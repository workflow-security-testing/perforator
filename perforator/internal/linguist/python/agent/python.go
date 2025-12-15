package agent

import (
	"fmt"
	"math"

	"github.com/yandex/perforator/perforator/agent/preprocessing/proto/python"
	"github.com/yandex/perforator/perforator/internal/unwinder"
)

func PythonInternalsOffsetsByVersion(version *python.PythonVersion) (*unwinder.PythonInternalsOffsets, error) {
	if version == nil {
		return nil, fmt.Errorf("nil version provided")
	}

	versionKey := encodeVersion(version)

	if offsets, ok := pythonVersionOffsets[versionKey]; ok {
		return offsets, nil
	}

	if offsets := findNearestVersionInMinor(version); offsets != nil {
		return offsets, nil
	}

	return nil, fmt.Errorf("no offsets available for Python %d.%d.%d", version.Major, version.Minor, version.Micro)
}

// findNearestVersionInMinor finds offsets for the nearest micro version
// within the same major.minor release. Returns nil if no suitable version found.
func findNearestVersionInMinor(targetVersion *python.PythonVersion) *unwinder.PythonInternalsOffsets {
	if targetVersion == nil {
		return nil
	}

	var nearestOffsets *unwinder.PythonInternalsOffsets
	minDistance := math.MaxInt

	for versionKey, offsets := range pythonVersionOffsets {
		decodedVersion := decodeVersion(versionKey)

		if decodedVersion.Major != targetVersion.Major || decodedVersion.Minor != targetVersion.Minor {
			continue
		}

		distance := int(targetVersion.Micro) - int(decodedVersion.Micro)
		if distance < 0 {
			distance = -distance
		}

		if distance < minDistance {
			minDistance = distance
			nearestOffsets = offsets
		}
	}

	return nearestOffsets
}

func IsVersionSupported(version *python.PythonVersion) bool {
	_, err := PythonInternalsOffsetsByVersion(version)
	return err == nil
}

// Only supported python version config must be passed here.
func ParsePythonUnwinderConfig(conf *python.PythonConfig) *unwinder.PythonConfig {
	offsets, _ := PythonInternalsOffsetsByVersion(conf.Version)
	return &unwinder.PythonConfig{
		Version:                     uint32(encodeVersion(conf.Version)),
		PyThreadStateTlsOffset:      uint64(-conf.PyThreadStateTLSOffset),
		PyRuntimeRelativeAddress:    conf.RelativePyRuntimeAddress,
		PyInterpHeadRelativeAddress: conf.RelativePyInterpHeadAddress,
		AutoTssKeyRelativeAddress:   conf.RelativeAutoTSSkeyAddress,
		UnicodeTypeSizeLog2:         conf.UnicodeTypeSizeLog2,
		Offsets:                     *offsets,
	}
}
