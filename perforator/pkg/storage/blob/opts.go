package blob

import (
	"github.com/yandex/perforator/perforator/pkg/s3"
)

type options struct {
	fsPath   string
	s3bucket string
	s3client *s3.Client
}

func defaultOpts() *options {
	return &options{}
}

type Option = func(o *options)

func WithS3(client *s3.Client, bucket string) Option {
	return func(o *options) {
		o.s3bucket = bucket
		o.s3client = client
	}
}

func WithFS(rootPath string) Option {
	return func(o *options) {
		o.fsPath = rootPath
	}
}
