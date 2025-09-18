package rate_limit

import (
	"context"

	"golang.org/x/time/rate"
	"google.golang.org/grpc"
)

type RateLimitInterceptor struct {
	limiters map[string]*rate.Limiter
}

func NewRateLimitInterceptor(config Config) *RateLimitInterceptor {
	limiters := make(map[string]*rate.Limiter)

	for _, method := range config.Methods {
		if method.AverageRPS > 0 && method.Path != "" {
			limiters[method.Path] = rate.NewLimiter(rate.Limit(method.AverageRPS), int(method.MaxRPS))
		}
	}

	return &RateLimitInterceptor{
		limiters: limiters,
	}
}

func (r *RateLimitInterceptor) UnaryInterceptor() grpc.UnaryClientInterceptor {
	return func(
		ctx context.Context,
		method string,
		req, reply interface{},
		cc *grpc.ClientConn,
		invoker grpc.UnaryInvoker,
		opts ...grpc.CallOption,
	) error {
		if limiter, ok := r.limiters[method]; ok {
			if err := limiter.Wait(ctx); err != nil {
				return err
			}
		}

		return invoker(ctx, method, req, reply, cc, opts...)
	}
}
