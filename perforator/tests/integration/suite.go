package integration

import (
	"github.com/stretchr/testify/suite"

	"github.com/yandex/perforator/perforator/pkg/xlog"
)

type IntegrationTestSuite struct {
	suite.Suite
	TestEnv *IntegrationTestEnv
}

func NewIntegrationTestSuite(l xlog.Logger, cfg *Config) *IntegrationTestSuite {
	return &IntegrationTestSuite{
		TestEnv: NewIntegrationTestEnv(l, cfg),
	}
}

func (s *IntegrationTestSuite) SetupSuite() {
	err := s.TestEnv.Start(s.T().Context())
	s.Require().NoError(err)
}

func (s *IntegrationTestSuite) TearDownSuite() {
	err := s.TestEnv.Finish(s.T().Context())
	s.Require().NoError(err)
}
