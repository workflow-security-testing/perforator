package services

import "google.golang.org/grpc"

type GRPCService interface {
	Register(server *grpc.Server) error
}
