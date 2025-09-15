package custom_profile

import (
	"context"
	"errors"

	"github.com/gofrs/uuid"
	"google.golang.org/protobuf/proto"

	blob "github.com/yandex/perforator/perforator/pkg/storage/blob/models"
	"github.com/yandex/perforator/perforator/pkg/storage/custom_profile/meta"
	"github.com/yandex/perforator/perforator/pkg/storage/util"
	"github.com/yandex/perforator/perforator/proto/profile"
)

var _ Storage = (*customProfileStorage)(nil)

type customProfileStorage struct {
	metaStorage meta.Storage
	blobStorage blob.Storage
}

func NewCustomProfileStorage(metaStorage meta.Storage, blobStorage blob.Storage) *customProfileStorage {
	return &customProfileStorage{
		metaStorage: metaStorage,
		blobStorage: blobStorage,
	}
}

func (s *customProfileStorage) putBlob(ctx context.Context, id string, bytes []byte) error {
	writer, err := s.blobStorage.Put(ctx, id)
	if err != nil {
		return err
	}

	_, err = writer.Write(bytes)
	if err != nil {
		return err
	}

	_, err = writer.Commit()
	return err
}

func (s *customProfileStorage) StoreCustomProfile(ctx context.Context, meta *meta.CustomProfileMeta, profile *profile.ProfileContainer) (string, error) {
	if meta == nil {
		return "", errors.New("meta is nil")
	}

	if meta.OperationID == "" {
		return "", errors.New("operationID is required")
	}

	id, err := uuid.NewV7()
	if err != nil {
		return "", err
	}

	meta.ID = id.String()

	profileBytes, err := proto.Marshal(profile)
	if err != nil {
		return "", err
	}

	err = s.putBlob(ctx, meta.ID, profileBytes)
	if err != nil {
		return "", err
	}

	err = s.metaStorage.StoreCustomProfile(ctx, meta)
	if err != nil {
		return "", err
	}

	return meta.ID, nil
}

func (s *customProfileStorage) GetOperationProfiles(ctx context.Context, operationID string) ([]*meta.CustomProfileMeta, error) {
	return s.metaStorage.GetOperationProfiles(ctx, operationID)
}

func (s *customProfileStorage) FetchProfile(ctx context.Context, meta *meta.CustomProfileMeta) (*profile.ProfileContainer, error) {
	buf := util.NewWriteAtBuffer(nil)
	err := s.blobStorage.Get(ctx, meta.ID, buf)
	if err != nil {
		return nil, err
	}

	var container profile.ProfileContainer
	err = proto.Unmarshal(buf.Bytes(), &container)
	if err != nil {
		return nil, err
	}

	return &container, nil
}
