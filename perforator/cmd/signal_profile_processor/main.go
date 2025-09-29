package main

import (
	"context"
	"fmt"
	"os"

	"github.com/spf13/cobra"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/library/go/core/log/zap"
	"github.com/yandex/perforator/perforator/internal/buildinfo/cobrabuildinfo"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/maxprocs"
	"github.com/yandex/perforator/perforator/pkg/mlock"
	"github.com/yandex/perforator/perforator/pkg/must"
	eventprocessor "github.com/yandex/perforator/perforator/pkg/profile_event/event_processor"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

var (
	configPath string
	logLevel   string
)

var rootCmd = &cobra.Command{
	Use:   "event-processor",
	Short: "Start signal profile event processor",
	RunE: func(_ *cobra.Command, _ []string) error {
		ctx, stop := context.WithCancel(context.Background())
		defer stop()

		level, err := log.ParseLevel(logLevel)
		if err != nil {
			return err
		}

		logger, err := xlog.TryNew(zap.NewDeployLogger(level))
		if err != nil {
			return err
		}

		err = mlock.LockExecutableMappings()
		if err == nil {
			logger.Info(ctx, "Locked self executable")
		} else {
			logger.Error(ctx, "Failed to lock self executable", log.Error(err))
		}

		conf, err := eventprocessor.ParseConfig(configPath)
		if err != nil {
			return err
		}

		reg := xmetrics.NewRegistry(
			xmetrics.WithAddCollectors(xmetrics.GetCollectFuncs()...),
		)

		svc, err := eventprocessor.NewService(logger, *conf, reg)
		if err != nil {
			return err
		}

		logger.Info(ctx, "Starting event processor",
			log.Int("queue_size", conf.QueueSize),
			log.Int("workers_number", conf.WorkersNumber),
			log.Int("whitelist_count", len(conf.WhitelistServices)),
		)

		err = svc.Run(ctx)
		return err
	},
}

func init() {
	rootCmd.Flags().StringVarP(
		&configPath,
		"config",
		"c",
		"",
		"Path to event processor config (YAML)",
	)
	must.Must(rootCmd.MarkFlagFilename("config"))

	rootCmd.Flags().StringVar(
		&logLevel,
		"log-level",
		"info",
		"Logging level - ('info') {'debug', 'info', 'warn', 'error'}",
	)

	cobrabuildinfo.Init(rootCmd)
}

func main() {
	maxprocs.Adjust()
	if err := rootCmd.Execute(); err != nil {
		fmt.Fprintf(os.Stderr, "Error: %+v\n", err)
		os.Exit(1)
	}
}
