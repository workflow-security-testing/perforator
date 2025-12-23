#pragma once

#ifdef __x86_64__

#include "arch/x86/unwind_ctx.h"

#elif __aarch64__

#include "arch/arm/unwind_ctx.h"

#else

#error This arch is not supported by Perforator yet

#endif
