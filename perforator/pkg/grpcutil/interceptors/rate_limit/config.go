package rate_limit

type RateLimitedMethod struct {
	Path       string `yaml:"path"`
	AverageRPS uint64 `yaml:"average_rps"`
	MaxRPS     uint64 `yaml:"max_rps"`
}

type Config struct {
	Methods []RateLimitedMethod `yaml:"methods"`
}
