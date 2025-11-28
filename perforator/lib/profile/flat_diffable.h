#pragma once

#include "profile.h"
#include "util/generic/function_ref.h"

#include <perforator/proto/pprofprofile/profile.pb.h>
#include <perforator/proto/profile/profile.pb.h>

#include <library/cpp/int128/int128.h>

#include <util/generic/hash_set.h>
#include <util/generic/map.h>


namespace NPerforator::NProfile {

struct TFlatDiffableProfileOptions {
    bool PrintTimestamps = true;
    bool PrintAddresses = true;
    bool PrintBuildIds = true;
    THashSet<TString> LabelBlacklist;
    // Profile labels here can contain empty string values.
    // Our behavior is intentionally different from pprof's:
    // PProf encodes a single label as one message containing either its string (str) or numeric (num)
    // part. If a label's value is set to zero (explicitly or defaulted), pprof only recognizes it if it's numeric
    // with a num_unit; otherwise, the label is ignored.
    // In contrast, our profile format encodes string and numeric labels as two separate lists,
    // allowing us to differentiate between them. We therefore exclude these labels from comparison here.
    // FIXME(ayles): Excluding string labels is also necessary for now, as our profiler
    // ignores well-known labels if they are zero, but pprof does not. We should consider
    // removing the special encoding for well-known labels altogether.
    bool PrintStringLabelsWithEmptyValues = true;
};

class TFlatDiffableProfile {
public:
    TFlatDiffableProfile(const NProto::NPProf::Profile& profile, TFlatDiffableProfileOptions options = {});
    TFlatDiffableProfile(const NProto::NProfile::Profile& profile, TFlatDiffableProfileOptions options = {});
    TFlatDiffableProfile(TProfile profile, TFlatDiffableProfileOptions options = {});

    void IterateSamples(TFunctionRef<void(TStringBuf key, const TMap<TString, ui64>& values)> consumer) const;
    void WriteTo(IOutputStream& out) const;

private:
    TMap<TString, TMap<TString, ui64>> Samples_;
};

} // namespace NPerforator::NProfile
