package symbolize

// #include <stdlib.h>
// #include <perforator/symbolizer/lib/symbolize/symbolizec.h>
import "C"
import (
	"context"
	"errors"
	"fmt"
	"time"
	"unsafe"

	pprof "github.com/google/pprof/profile"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/metrics"
	"github.com/yandex/perforator/perforator/internal/symbolizer/binaryprovider"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	"github.com/yandex/perforator/perforator/proto/perforator"
	"github.com/yandex/perforator/perforator/proto/symbolizer"
)

func newLineInfo(buildID string, addr uint64, lineInfo *C.TLineInfo) *LineInfo {
	return &LineInfo{
		BuildID: buildID,
		Address: addr,
		ProtoLine: &ProtoLine{
			DemangledFunctionName: C.GoString(lineInfo.DemangledFunctionName),
			FunctionName:          C.GoString(lineInfo.FunctionName),
			Filename:              C.GoString(lineInfo.FileName),
			StartLine:             uint64(lineInfo.StartLine),
			Line:                  uint64(lineInfo.Line),
			Column:                uint64(lineInfo.Column),
			Discriminator:         uint64(lineInfo.Discriminator),
		},
	}
}

func NewSymbolizer(
	logger xlog.Logger,
	reg metrics.Registry,
	binaryProvider binaryprovider.BinaryProvider,
	gsymBinaryProvider binaryprovider.BinaryProvider,
	symbolizationMode SymbolizationMode,
) (*Symbolizer, error) {
	var errPtr *C.char = nil
	var symbolizer unsafe.Pointer = C.MakeSymbolizer(&errPtr)
	if errPtr != nil {
		return nil, errors.New(C.GoString(errPtr))
	}

	reg = reg.WithPrefix("symbolizer")

	return &Symbolizer{
		logger:             logger,
		symbolizationMode:  symbolizationMode,
		binaryProvider:     binaryProvider,
		gsymBinaryProvider: gsymBinaryProvider,
		symbolizer:         symbolizer,
		metrics: &symbolizerMetrics{
			symbolizationTimer:        reg.Timer("symbolization.timer"),
			unknownBinaries:           reg.Counter("unknown_binaries.count"),
			unsymbolizableLocations:   reg.Counter("unsymbolizable_locations.count"),
			binariesWithDWARFFallback: reg.Counter("binaries_with_dwarf_fallback.count"),
			binariesWithGSYM:          reg.Counter("binaries_with_gsym.count"),
		},
	}, nil
}

func (s *Symbolizer) Destroy() {
	C.DestroySymbolizer(s.symbolizer)
}

func (s *Symbolizer) symbolizePprof(
	ctx context.Context,
	profile *pprof.Profile,
	pathProvider BinaryPathProvider,
	gsymPathProvider BinaryPathProvider,
	opts *perforator.SymbolizeOptions,
) error {
	start := time.Now()
	defer func() {
		C.PruneCaches(s.symbolizer)
		s.metrics.symbolizationTimer.RecordDuration(time.Since(start))
	}()

	s.logger.Debug(ctx, "Start symbolize")
	for _, location := range profile.Location {
		// Skip symbolized code.
		if len(location.Line) > 0 {
			continue
		}

		if location.Mapping == nil {
			continue
		}

		path := pathProvider.PathByBuildID(location.Mapping.BuildID)
		address := location.Address + location.Mapping.Offset - location.Mapping.Start

		useGSYM := false
		gsymPath := gsymPathProvider.PathByBuildID(location.Mapping.BuildID)
		if gsymPath != "" {
			path = gsymPath
			useGSYM = true
		}

		if path == "" {
			s.logger.Trace(ctx, "Unknown binary",
				log.String("buildid", location.Mapping.BuildID),
				log.String("address", fmt.Sprintf("%x", location.Address)),
				log.String("original_file", location.Mapping.File),
			)
			s.metrics.unknownBinaries.Inc()
			continue
		}

		lineInfos, _ := s.symbolizeLocation(ctx, location.Mapping.BuildID, address, path, useGSYM)
		for _, lineInfo := range lineInfos {
			AddLine(profile, location, (*symbolizer.Line)(lineInfo.ProtoLine), opts)
		}
	}

	return nil
}

func (s *Symbolizer) symbolizeLocation(
	ctx context.Context,
	buildID string,
	address uint64,
	path string,
	useGSYM bool,
) ([]*LineInfo, error) {
	cUseGSYM := C.ui32(0)
	if useGSYM {
		cUseGSYM = C.ui32(1)
	}

	cpath := C.CString(path)
	defer C.free(unsafe.Pointer(cpath))
	linesCount := C.ui64(0)
	var errPtr *C.char = nil

	lines := C.Symbolize(
		s.symbolizer,
		cpath,
		C.ulong(len(path)),
		C.ui64(address),
		&linesCount,
		&errPtr,
		cUseGSYM,
	)
	if errPtr != nil {
		errStr := C.GoString(errPtr)
		s.logger.Error(ctx, "Failed to symbolize code",
			log.String("error", errStr),
			log.String("build_id", buildID),
			log.String("path", path),
			log.UInt64("address", address),
			log.Bool("useGSYM", useGSYM))
		s.metrics.unsymbolizableLocations.Inc()
		return nil, errors.New(errStr)
	}
	defer C.DestroySymbolizeResult(lines, linesCount)

	if linesCount == 0 {
		return nil, nil
	}

	res := make([]*LineInfo, linesCount)
	linesSlice := unsafe.Slice(lines, linesCount)
	for i, lineInfo := range linesSlice {
		res[i] = newLineInfo(buildID, address, &lineInfo)
	}

	return res, nil
}
