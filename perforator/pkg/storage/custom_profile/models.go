package custom_profile

import (
	"context"

	"github.com/yandex/perforator/perforator/pkg/storage/custom_profile/meta"
	"github.com/yandex/perforator/perforator/proto/profile"
)

type Storage interface {
	StoreCustomProfile(
		ctx context.Context,
		meta *meta.CustomProfileMeta,
		profile *profile.ProfileContainer,
	) (profileID string, err error)

	GetOperationProfiles(
		ctx context.Context,
		operationID string,
	) ([]*meta.CustomProfileMeta, error)

	FetchProfile(
		ctx context.Context,
		meta *meta.CustomProfileMeta,
	) (*profile.ProfileContainer, error)
}
