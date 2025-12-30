#include "visitor.h"

namespace NPerforator::NProfile {

void VisitProfile(
    const NProto::NProfile::Profile& profile,
    IProfileVisitor& visitor
) {
    visitor.VisitWholeProfile(profile);
    visitor.VisitStringTable(profile.strtab());
    visitor.VisitMetadata(profile.metadata());
    visitor.VisitComments(profile.comments());
    visitor.VisitLabels(profile.labels());
    visitor.VisitLabelGroups(profile.label_groups());
    visitor.VisitBinaries(profile.binaries());
    visitor.VisitFunctions(profile.functions());
    visitor.VisitInlineChains(profile.inline_chains());
    visitor.VisitStackFrames(profile.stack_frames());
    visitor.VisitStackSegments(profile.stack_segments());
    visitor.VisitStacks(profile.stacks());
    visitor.VisitSampleKeys(profile.sample_keys());
    visitor.VisitSamples(profile.samples());
}

} // namespace NPerforator::NProfile
