package compound

import "github.com/yandex/perforator/perforator/pkg/storage/custom_profile/meta"

type Config struct {
	MetaStorageType meta.CustomProfileStorageType `yaml:"meta_storage_type"`
	S3Bucket        string                        `yaml:"bucket"`
}
