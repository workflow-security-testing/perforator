package services

import (
	"context"

	"github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
	"google.golang.org/grpc"
)

type GRPCService interface {
	Register(server *grpc.Server) error
	RegisterHandler(ctx context.Context, mux *runtime.ServeMux) error
}
