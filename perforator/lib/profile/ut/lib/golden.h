#pragma once

#include <perforator/lib/profile/flat_diffable.h>

#include <library/cpp/iterator/zip.h>
#include <library/cpp/testing/common/env.h>
#include <library/cpp/testing/gtest/gtest.h>

#include <util/generic/maybe.h>
#include <util/string/split.h>


namespace NPerforator::NProfile::NTest {

TString DecompressPprof(const TFsPath& path);

TVector<TFsPath> ListGoldenProfiles(const TFsPath& path, TStringBuf pattern, TMaybe<size_t> expectedProfileCount);

TString SerializeFlatProfile(const TFlatDiffableProfile& profile);

TMap<TString, ui64> CountFlatProfileEvents(const TFlatDiffableProfile& profile);

template <typename L, typename R>
void CompareFlatProfiles(const L& lhs, const R& rhs, TFlatDiffableProfileOptions options = {}) {
    // Our profiles are somewhat malformed
    options.LabelBlacklist.emplace("comm");
    TFlatDiffableProfile expected{lhs, options};
    TFlatDiffableProfile actual{rhs, options};

    auto expectedEvents = CountFlatProfileEvents(expected);
    auto actualEvents = CountFlatProfileEvents(actual);
    EXPECT_EQ(expectedEvents, actualEvents);

    TString expectedString = SerializeFlatProfile(expected);
    TString actualString = SerializeFlatProfile(actual);

    // *_EQ for strings will try to compute edit distance for pretty diffs,
    // but we have very long strings.
    size_t lineIndex = 0;
    for (auto&& [expectedLine, actualLine] : Zip(StringSplitter(expectedString).Split('\n'), StringSplitter(actualString).Split('\n'))) {
        ASSERT_EQ(std::string_view{expectedLine}, std::string_view{actualLine}) << "Lines with index " << lineIndex << " differ";
        lineIndex++;
    }

    // Check that we didn't miss any excess lines.
    EXPECT_TRUE(expectedString.size() == actualString.size());
}

} // namespace NPerforator::NProfile::NTest
