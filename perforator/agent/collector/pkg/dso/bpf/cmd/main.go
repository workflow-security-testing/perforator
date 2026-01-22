package main

import (
	"context"
	"flag"
	"os"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/log/zap"
	"github.com/yandex/perforator/library/go/core/metrics/mock"
	"github.com/yandex/perforator/perforator/agent/collector/pkg/dso/bpf/binary"
	"github.com/yandex/perforator/perforator/agent/collector/pkg/dso/parser"
	"github.com/yandex/perforator/perforator/agent/preprocessing/proto/parse"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

func main() {
	l := zap.Must(zap.TSKVConfig(log.DebugLevel))

	if err := run(context.Background(), xlog.New(l)); err != nil {
		l.Fatal("Failed to analyze binary", log.Error(err))
	}
}

func run(ctx context.Context, l xlog.Logger) error {
	r := mock.NewRegistry(nil)

	binaryParserOptions := &parse.BinaryAnalysisOptions{}
	var preferSframe bool
	flag.BoolVar(&preferSframe, "prefer-sframe", false, "")
	if preferSframe {
		binaryParserOptions.PreferredUnwindInfoSource = parse.UnwindInfoSource_Sframe
	}

	m, err := binary.NewBPFBinaryManager(l.Logger(), r, nil /* state */)
	if err != nil {
		return err
	}

	binaryParser, err := parser.NewBinaryParser(l, r, binaryParserOptions)
	if err != nil {
		return err
	}

	f, err := os.Open(flag.Args()[0])
	if err != nil {
		return err
	}
	defer f.Close()

	analysis, err := binaryParser.Parse(ctx, f)
	if err != nil {
		return err
	}

	a, err := m.Add(ctx, "nobuildid", 0, analysis)
	if err != nil {
		return err
	}

	l.Info(ctx, "Analyzed binary", log.Any("allocation", a))
	return nil
}
