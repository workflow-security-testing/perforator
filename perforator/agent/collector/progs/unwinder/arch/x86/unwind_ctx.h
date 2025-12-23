#pragma once

#include <bpf/bpf.h>

struct unwind_context {
    u64 cfa;
    u64 fp;
    u64 ip;
};
