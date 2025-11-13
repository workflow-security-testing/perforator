#pragma once

#include "funcs.h"

#define Q0(X) #X

#define Q(X) Q0(X)

#define BPF_PRINTK(fmt, ...)                                               \
    ({                                                                     \
     static const char __fmt[] = "[perforator] " __FILE__ ":" Q(__LINE__) " " fmt;         \
     bpf_trace_printk(__fmt, sizeof(__fmt), ##__VA_ARGS__);                \
     })

#ifdef BPF_DEBUG

#define BPF_TRACE BPF_PRINTK

#else // BPF_DEBUG

#define BPF_TRACE(...)

#endif // BPF_DEBUG
