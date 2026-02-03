#pragma once

#include "error.h"
#include "string.h"
#include "profile.h"

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////

// Render flamegraph as JSON
// Options passed as serialized proto (NPerforator::NProto::NProfile::RenderOptions)
TPerforatorError PerforatorRenderFlameGraph(
    TPerforatorProfile profile,
    const char* optionsPtr,
    size_t optionsSize,
    TPerforatorString* result);

// Render flamegraph from pprof as JSON
// Options passed as serialized proto (NPerforator::NProto::NProfile::RenderOptions)
TPerforatorError PerforatorRenderFlameGraphFromPProf(
    const char* ptr,
    size_t size,
    const char* optionsPtr,
    size_t optionsSize,
    TPerforatorString* result);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
