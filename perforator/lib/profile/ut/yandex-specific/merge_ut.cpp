#include <perforator/lib/profile/flat_diffable.h>
#include <perforator/lib/profile/merge_manager.h>
#include <perforator/lib/profile/pprof.h>
#include <perforator/lib/profile/ut/lib/golden.h>

#include <library/cpp/testing/gtest/gtest.h>
#include <library/cpp/testing/common/env.h>

#include <util/stream/file.h>

using namespace NPerforator::NProfile::NTest;

TEST(MergeProfilesTest, GoldenBig) {
    TVector<TString> profilesBytes;
    NPerforator::NProto::NPProf::Profile expected;

    for (TFsPath path : NPerforator::NProfile::NTest::ListGoldenProfiles(BuildRoot() / TFsPath(__SOURCE_FILE__).Parent() / "testprofiles" / "merge_yabs", ".*.pb.gz", 4)) {
        TFileInput input{path};

        auto profileBytes = DecompressPprof(path);
        if (path.GetName().StartsWith("merged")) {
            expected.ParseFromStringOrThrow(profileBytes);
        } else {
            profilesBytes.emplace_back(std::move(profileBytes));
        }
    }

    Y_ENSURE(profilesBytes.size() > 2);
    Y_ENSURE(expected.sample_size() > 100);

    const ui32 threadCount = 4;
    NPerforator::NProfile::TMergeManager manager{threadCount};

    NPerforator::NProto::NProfile::MergeOptions options;
    options.set_ignore_process_ids(false);
    options.set_ignore_thread_ids(false);
    options.set_cleanup_thread_names(false);
    auto session = manager.StartSession(options);

    for (auto&& profileBytes : profilesBytes) {
        NPerforator::NProto::NProfile::Profile profile;
        NPerforator::NProfile::ConvertFromPProf(profileBytes, &profile);
        session->AddProfile(std::move(profile));
    }
    auto merged = std::move(*session).Finish();

    CompareFlatProfiles</*Big=*/true>(expected, merged, NPerforator::NProfile::TFlatDiffableProfileOptions{.PrintStringLabelsWithEmptyValues = false});
}
