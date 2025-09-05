package agent_gateway

import (
	"context"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	custom_profiling_operation_proto "github.com/yandex/perforator/perforator/proto/custom_profiling_operation"
)

type customProfilingOperationService struct {
}

func newCustomProfilingOperationService() (*customProfilingOperationService, error) {
	return &customProfilingOperationService{}, nil
}

func (s *customProfilingOperationService) PollOperations(ctx context.Context, req *custom_profiling_operation_proto.PollOperationsRequest) (*custom_profiling_operation_proto.PollOperationsResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method PollOperations not implemented")
}

func (s *customProfilingOperationService) UpdateOperationExecutionInfo(ctx context.Context, req *custom_profiling_operation_proto.UpdateOperationExecutionInfoRequest) (*custom_profiling_operation_proto.UpdateOperationExecutionInfoResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method UpdateOperationExecutionInfo not implemented")
}
