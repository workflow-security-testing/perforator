#include <perforator/lib/profile/flat_diffable.h>
#include <perforator/lib/profile/pprof.h>
#include <perforator/lib/profile/profile.h>
#include <perforator/lib/profile/ut/lib/golden.h>

#include <library/cpp/testing/gtest/gtest.h>
#include <library/cpp/testing/common/env.h>


using namespace NPerforator::NProfile::NTest;

// NOLINTNEXTLINE(readability-identifier-naming)
struct GoldenProfileTest : testing::TestWithParam<TFsPath> {};

TEST_P(GoldenProfileTest, ConvertPprofCanon) {
    NPerforator::NProto::NPProf::Profile pprofProto;
    Y_ENSURE(pprofProto.ParseFromString(DecompressPprof(GetParam())));

    // Make new protobuf profile from pprof
    NPerforator::NProto::NProfile::Profile profileProto;
    NPerforator::NProfile::ConvertFromPProf(pprofProto, &profileProto);

    CompareFlatProfiles(pprofProto, profileProto);
}

TEST_P(GoldenProfileTest, ConvertPprofRoundTripCanon) {
    NPerforator::NProto::NPProf::Profile pprofOriginalProto;
    Y_ENSURE(pprofOriginalProto.ParseFromString(DecompressPprof(GetParam())));

    // Make new protobuf profile from pprof
    NPerforator::NProto::NProfile::Profile profileProto;
    NPerforator::NProfile::ConvertFromPProf(pprofOriginalProto, &profileProto);

    NPerforator::NProto::NPProf::Profile pprofConvertedProto;
    NPerforator::NProfile::ConvertToPProf(profileProto, &pprofConvertedProto);

    CompareFlatProfiles(pprofOriginalProto, pprofConvertedProto);
}

TEST_P(GoldenProfileTest, ConvertPprofBytesCanon) {
    TString pprofBytes = DecompressPprof(GetParam());

    NPerforator::NProto::NProfile::Profile profileProto;
    NPerforator::NProfile::ConvertFromPProf(pprofBytes, &profileProto);

    NPerforator::NProto::NPProf::Profile pprof;
    Y_ENSURE(pprof.ParseFromString(pprofBytes));

    CompareFlatProfiles(pprof, profileProto);
}

TEST_P(GoldenProfileTest, ConvertPprofBytesRoundTripCanon) {
    TString pprofOriginalBytes = DecompressPprof(GetParam());

    NPerforator::NProto::NProfile::Profile profileProto;
    NPerforator::NProfile::ConvertFromPProf(pprofOriginalBytes, &profileProto);

    NPerforator::NProto::NPProf::Profile pprofConvertedProto;
    NPerforator::NProfile::ConvertToPProf(profileProto, &pprofConvertedProto);

    NPerforator::NProto::NPProf::Profile pprofOriginalProto;
    Y_ENSURE(pprofOriginalProto.ParseFromString(pprofOriginalBytes));

    CompareFlatProfiles(pprofOriginalProto, pprofConvertedProto);
}

INSTANTIATE_TEST_SUITE_P(
    GoldenProfiles,
    GoldenProfileTest,
    testing::ValuesIn(NPerforator::NProfile::NTest::ListGoldenProfiles(SRC_("testprofiles/diff"), ".*.pb.gz", 5))
);
