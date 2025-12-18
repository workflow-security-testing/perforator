#pragma once

#if defined(__aarch64__)

#include "arm/decode.h"

#elif defined(__x86_64__)

#include "x86/decode.h"

#else

#error "Unsupported architecture"

#endif

namespace NPerforator::NLinguist::NPhp::NAsm {

#if defined(__aarch64__)

using NArm::DecodePhpVersion;
using NArm::DecodeZmInfoPhpCore;
using NArm::DecodeZendVmKind;

#elif defined(__x86_64__)

using NX86::DecodePhpVersion;
using NX86::DecodeZmInfoPhpCore;
using NX86::DecodeZendVmKind;

#endif

}
