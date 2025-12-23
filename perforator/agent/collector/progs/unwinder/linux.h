#pragma once

#ifndef __LINUX_PAGE_CONSTANTS_HACK__
#define __LINUX_PAGE_CONSTANTS_HACK__

// Values for x86_64 as of 6.0.18-200.
#ifdef __x86_64__

#define TOP_OF_KERNEL_STACK_PADDING 0
#define THREAD_SIZE_ORDER 2
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define THREAD_SIZE (PAGE_SIZE << THREAD_SIZE_ORDER)

struct thread_struct {
    unsigned long fsbase;
};

#elif __aarch64__

#include <bpf/types.h>

// Frame Pointer, same as x86 rbp
// See: https://elixir.bootlin.com/linux/v5.4.254/source/arch/arm64/include/asm/ptrace.h#L358
#define ARM_FP regs[29]
// Return register
// See: https://elixir.bootlin.com/linux/v5.4.254/source/arch/arm64/include/asm/ptrace.h#L358
#define ARM_LR regs[30]

#define THREAD_SIZE_ORDER 1
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define THREAD_SIZE (PAGE_SIZE << THREAD_SIZE_ORDER)
#define THREAD_START_SP (THREAD_SIZE - 8)

// FIXME(lexmach): somehow linux-headers doesn't have pt_regs on arm64.
struct pt_regs {
    struct user_pt_regs user_regs;
};

// See: https://elixir.bootlin.com/linux/v5.4.254/source/arch/arm64/include/asm/processor.h#L125
struct thread_struct {
    struct {
        unsigned long tp_value;
    } uw;
};

#else

#error This arch is not supported by Perforator yet

#endif

#endif // __LINUX_PAGE_CONSTANTS_HACK__
