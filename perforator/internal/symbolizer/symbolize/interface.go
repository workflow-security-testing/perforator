package symbolize

import (
	"github.com/yandex/perforator/perforator/pkg/xelf"
)

type BinaryPathProvider interface {
	PathByBuildID(buildID string) string
}

type fixedBinariesPathProvider struct {
	binaryPathByBuildID map[string]string
}

func (p *fixedBinariesPathProvider) PathByBuildID(buildId string) string {
	if path, ok := p.binaryPathByBuildID[buildId]; ok {
		return path
	}
	return ""
}

func NewFixedBinariesPathProvider(binaryPaths []string) (BinaryPathProvider, error) {
	binaryPathByBuildID := make(map[string]string)
	for _, binaryPath := range binaryPaths {
		buildID, err := xelf.GetBuildID(binaryPath)
		if err != nil {
			return nil, err
		}
		binaryPathByBuildID[buildID] = binaryPath
	}
	return &fixedBinariesPathProvider{
		binaryPathByBuildID: binaryPathByBuildID,
	}, nil
}

type nilPathProvider struct{}

func (*nilPathProvider) PathByBuildID(buildId string) string {
	return ""
}

func NewNilPathProvider() BinaryPathProvider {
	return &nilPathProvider{}
}

var _ BinaryPathProvider = (*nilPathProvider)(nil)
var _ BinaryPathProvider = (*fixedBinariesPathProvider)(nil)
