package cmd

import (
	"context"

	"github.com/spf13/cobra"

	"github.com/yandex/perforator/library/go/core/log"
	"github.com/yandex/perforator/perforator/internal/offline_processing/app"
	"github.com/yandex/perforator/perforator/internal/xmetrics"
	"github.com/yandex/perforator/perforator/pkg/mlock"
	"github.com/yandex/perforator/perforator/pkg/must"
	"github.com/yandex/perforator/perforator/pkg/xlog"
)

var (
	logLevelStr string
	configPath  string

	appCommand = &cobra.Command{
		Use:   "app",
		Short: "Run the background-processing app",
		RunE: func(cmd *cobra.Command, args []string) error {
			ctx := context.Background()

			reg := xmetrics.NewRegistry(
				xmetrics.WithAddCollectors(xmetrics.GetCollectFuncs()...),
			)

			logLevel, err := log.ParseLevel(logLevelStr)
			if err != nil {
				return err
			}

			logger, stopLogger, err := xlog.ForDaemon(xlog.DaemonConfig{Level: logLevel}, reg)
			if err != nil {
				return err
			}
			defer stopLogger()

			err = mlock.LockExecutableMappings()
			if err == nil {
				logger.Info(ctx, "Locked self executable")
			} else {
				logger.Error(ctx, "Failed to lock self executable", log.Error(err))
			}

			conf, err := app.ParseConfig(configPath)
			if err != nil {
				return err
			}

			app, err := app.NewOfflineProcessingApp(conf, logger, reg)
			if err != nil {
				return err
			}

			return app.Run(ctx)
		},
	}
)

func init() {
	appCommand.Flags().StringVarP(
		&configPath,
		"config",
		"c",
		"",
		"Path to offiline-processing app config",
	)
	must.Must(appCommand.MarkFlagFilename("config"))

	appCommand.Flags().StringVar(
		&logLevelStr,
		"log-level",
		"info",
		"Logging level - ('info') {'debug', 'info', 'warn', 'error'}",
	)

	rootCmd.AddCommand(appCommand)
}
