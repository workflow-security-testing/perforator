package meta

import (
	"context"
	"time"
)

type CustomProfileStorageType = string

const (
	Clickhouse CustomProfileStorageType = "clickhouse"
)

type CustomProfileMeta struct {
	ID            string
	OperationID   string
	FromTimestamp time.Time
	ToTimestamp   time.Time
	BuildIDs      []string
	Labels        map[string]string
}

type Storage interface {
	StoreCustomProfile(
		ctx context.Context,
		meta *CustomProfileMeta,
	) error

	GetOperationProfiles(
		ctx context.Context,
		operationID string,
	) ([]*CustomProfileMeta, error)
}
