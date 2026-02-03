#pragma once

#include <perforator/proto/profile/profile.pb.h>
#include <perforator/proto/profile/render_options.pb.h>

#include <util/stream/output.h>

namespace NPerforator::NProfile {

////////////////////////////////////////////////////////////////////////////////

// Render flamegraph as JSON
// Performs profile merging internally with standard refinements
void RenderFlameGraphJson(
    const NProto::NProfile::Profile& profile,
    IOutputStream& out,
    const NProto::NProfile::RenderOptions& options = {}
);

////////////////////////////////////////////////////////////////////////////////

} // namespace NPerforator::NProfile
