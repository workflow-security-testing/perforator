package service

import (
	"errors"
	"time"

	"go.opentelemetry.io/contrib/instrumentation/google.golang.org/grpc/otelgrpc"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/keepalive"

	"github.com/yandex/perforator/perforator/pkg/xlog"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

////////////////////////////////////////////////////////////////////////////////

type ClientConfig struct {
	HTTPHost string `yaml:"http_host"`
	GRPCHost string `yaml:"grpc_host"`

	// TODO: add OAuth token with perforator:api scope and tls support.
	//Insecure bool
	//Token    string
}

type Client struct {
	l xlog.Logger

	connection *grpc.ClientConn

	perforator.PerforatorClient
	perforator.MicroscopeServiceClient
	perforator.TaskServiceClient
	perforator.ClusterTopClient
}

func NewClient(cfg *ClientConfig, l xlog.Logger) (*Client, error) {
	if cfg.GRPCHost == "" {
		return nil, errors.New("grpc host is not set")
	}

	opts := []grpc.DialOption{
		grpc.WithKeepaliveParams(keepalive.ClientParameters{
			Time: 30 * time.Second,
		}),
		grpc.WithDefaultCallOptions(grpc.MaxCallRecvMsgSize(1024 * 1024 * 1024 /*1G*/)),
		grpc.WithStatsHandler(otelgrpc.NewClientHandler()),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	}

	conn, err := grpc.Dial(cfg.GRPCHost, opts...)
	if err != nil {
		return nil, err
	}

	return &Client{
		l:                       l,
		connection:              conn,
		PerforatorClient:        perforator.NewPerforatorClient(conn),
		MicroscopeServiceClient: perforator.NewMicroscopeServiceClient(conn),
		TaskServiceClient:       perforator.NewTaskServiceClient(conn),
		ClusterTopClient:        perforator.NewClusterTopClient(conn),
	}, nil
}
