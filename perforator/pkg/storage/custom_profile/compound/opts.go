package compound

import (
	"github.com/aws/aws-sdk-go/service/s3"

	"github.com/yandex/perforator/perforator/pkg/clickhouse"
)

type options struct {
	clickhouseConn *clickhouse.Connection
	s3client       *s3.S3
	s3bucket       string
}

func defaultOpts() *options {
	return &options{}
}

type Option = func(o *options)

func WithClickhouseMetaStorage(conn *clickhouse.Connection) Option {
	return func(o *options) {
		o.clickhouseConn = conn
	}
}

func WithS3(client *s3.S3, bucket string) Option {
	return func(o *options) {
		o.s3bucket = bucket
		o.s3client = client
	}
}
