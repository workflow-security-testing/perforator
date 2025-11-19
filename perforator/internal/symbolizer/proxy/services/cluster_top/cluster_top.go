package cluster_top

import (
	"context"
	_ "net/http/pprof"

	"github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/yandex/perforator/perforator/internal/symbolizer/proxy/services"
	clickhouse "github.com/yandex/perforator/perforator/pkg/storage/cluster_top"
	"github.com/yandex/perforator/perforator/pkg/storage/cluster_top/aggregated"
	"github.com/yandex/perforator/perforator/pkg/xlog"
	"github.com/yandex/perforator/perforator/proto/perforator"
)

var generationArgumentError = status.Errorf(codes.InvalidArgument, "Must provide non-zero generation")

var (
	_ services.GRPCService = (*APIService)(nil)
)

type APIService struct {
	l                           xlog.Logger
	clusterTopGenerationStorage clickhouse.Storage
}

func NewService(l xlog.Logger, s clickhouse.Storage) *APIService {

	return &APIService{
		l:                           l,
		clusterTopGenerationStorage: s,
	}
}

// GetClusterTopAggregatedByFunction implements perforator.GetClusterTopAggregatedByFunction
func (s *APIService) GetClusterTopAggregatedByFunction(ctx context.Context, req *perforator.ClusterTopRequest) (*perforator.ClusterTopResponse, error) {
	if req.GetGeneration() == 0 {
		return nil, generationArgumentError
	}

	return s.clusterTopGenerationStorage.AggregateClusterTop(ctx, req.GetGeneration(), "", aggregated.GroupByFunction)
}

// GetClusterTopAggregatedByService implements perforator.GetClusterTopAggregatedByService
func (s *APIService) GetClusterTopAggregatedByService(ctx context.Context, req *perforator.ClusterTopRequest) (*perforator.ClusterTopResponse, error) {
	if req.GetGeneration() == 0 {
		return nil, generationArgumentError
	}
	if req.FunctionPattern == nil {
		return nil, status.Errorf(codes.InvalidArgument, "For service aggregation must provide non-empty function search pattern")
	}

	return s.clusterTopGenerationStorage.AggregateClusterTop(ctx, req.GetGeneration(), req.GetFunctionPattern(), aggregated.GroupByService)
}

// ListClusterTopGenerations implements perforator.ListClusterTopGenerations
func (s *APIService) ListClusterTopGenerations(ctx context.Context, req *perforator.ListClusterTopGenerationRequest) (*perforator.ListClusterTopGenerationResponse, error) {
	fields, err := s.clusterTopGenerationStorage.ListGenerations(ctx)
	return &perforator.ListClusterTopGenerationResponse{
		Generations: fields,
	}, err
}

func (s *APIService) Register(server *grpc.Server) error {
	perforator.RegisterClusterTopServer(server, s)
	return nil
}

func (s *APIService) RegisterHandler(ctx context.Context, mux *runtime.ServeMux) error {
	return perforator.RegisterClusterTopHandlerServer(ctx, mux, s)
}
