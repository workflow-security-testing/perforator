package symbolize

import "github.com/yandex/perforator/perforator/proto/symbolizer"

// since field `Line` conflicts with
// embedded type name `symbolizer.Line`
type ProtoLine = symbolizer.Line

// Representation of C.TLineInfo
type LineInfo struct {
	BuildID string
	Address uint64
	*ProtoLine
}
