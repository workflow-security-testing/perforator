#include "flamegraph.h"

#include "error.hpp"
#include "profile.hpp"
#include "string.hpp"

#include <perforator/lib/profile/flamegraph/render.h>
#include <perforator/lib/profile/pprof.h>
#include <perforator/proto/profile/render_options.pb.h>

#include <util/stream/str.h>


namespace NPerforator::NProfile::NCWrapper {

namespace {

NProto::NProfile::RenderOptions ParseRenderOptions(const char* ptr, size_t size) {
    NProto::NProfile::RenderOptions options;
    if (ptr != nullptr && size > 0) {
        Y_ENSURE(options.ParseFromArray(ptr, size), "Failed to parse render options");
    }
    return options;
}

} // namespace

extern "C" {

TPerforatorError PerforatorRenderFlameGraph(
    TPerforatorProfile profile,
    const char* optionsPtr,
    size_t optionsSize,
    TPerforatorString* result
) {
    return InterceptExceptions([&] {
        auto renderOptions = ParseRenderOptions(optionsPtr, optionsSize);
        TStringStream out;
        RenderFlameGraphJson(*UnwrapProfile(profile), out, renderOptions);
        *result = MakeString(out.Str());
    });
}

TPerforatorError PerforatorRenderFlameGraphFromPProf(
    const char* ptr,
    size_t size,
    const char* optionsPtr,
    size_t optionsSize,
    TPerforatorString* result
) {
    return InterceptExceptions([&] {
        NProto::NProfile::Profile proto;
        ConvertFromPProf(TStringBuf{ptr, size}, &proto);

        auto renderOptions = ParseRenderOptions(optionsPtr, optionsSize);
        TStringStream out;
        RenderFlameGraphJson(proto, out, renderOptions);
        *result = MakeString(out.Str());
    });
}

} // extern "C"

} // namespace NPerforator::NProfile::NCWrapper
