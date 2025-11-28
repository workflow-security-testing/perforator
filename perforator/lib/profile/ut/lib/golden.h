#pragma once

#include <perforator/lib/profile/flat_diffable.h>

#include <library/cpp/testing/common/env.h>
#include <library/cpp/testing/gtest/gtest.h>

#include <util/generic/maybe.h>


namespace NPerforator::NProfile::NTest {

TString DecompressPprof(const TFsPath& path);

TVector<TFsPath> ListGoldenProfiles(const TFsPath& path, TStringBuf pattern, TMaybe<size_t> expectedProfileCount);

TString SerializeFlatProfile(const TFlatDiffableProfile& profile);

TMap<TString, ui64> CountFlatProfileEvents(const TFlatDiffableProfile& profile);

template <bool Big = false, typename L, typename R>
void CompareFlatProfiles(const L& lhs, const R& rhs, TFlatDiffableProfileOptions options = {}) {
    // Our profiles are somewhat malformed
    options.LabelBlacklist.emplace("comm");
    TFlatDiffableProfile left{lhs, options};
    TFlatDiffableProfile right{rhs, options};

    auto lhsEvents = CountFlatProfileEvents(left);
    auto rhsEvents = CountFlatProfileEvents(right);
    EXPECT_EQ(lhsEvents, rhsEvents);

    TString pprofString = SerializeFlatProfile(left);
    TString protoString = SerializeFlatProfile(right);
    if constexpr (Big) {
        // EXPECT_EQ for strings will try to compute edit distance for pretty diffs,
        // but we have very long strings.
        // TODO(ayles): compare line by line?
        EXPECT_TRUE(pprofString == protoString);
    } else {
        EXPECT_EQ(pprofString, protoString);
    }
}

} // namespace NPerforator::NProfile::NTest
