GO_LIBRARY()

PEERDIR(
    perforator/pkg/storage/custom_profile
    perforator/pkg/storage/custom_profile/meta
    perforator/proto/profile
)

GO_MOCKGEN_FROM(perforator/pkg/storage/custom_profile)
GO_MOCKGEN_SOURCE(models.go)

END()
