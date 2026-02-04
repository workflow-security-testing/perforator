package agent

import "github.com/yandex/perforator/perforator/agent/preprocessing/proto/python"

type encodedVersion uint32

func encodeVersion(version *python.PythonVersion) encodedVersion {
	return encodedVersion(version.Micro + (version.Minor << 8) + (version.Major << 16))
}

func decodeVersion(encoded encodedVersion) *python.PythonVersion {
	return &python.PythonVersion{
		Micro: uint32(encoded & 0xFF),
		Minor: uint32((encoded >> 8) & 0xFF),
		Major: uint32(encoded >> 16),
	}
}
