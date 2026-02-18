#include <library/cpp/getopt/last_getopt.h>

#include <array>
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

#include <sched.h>
#include <unistd.h>

static std::atomic<bool> gRunning{true};

// The target function that will be uprobed.
// It must be noinline so that every call generates a uprobe event.
// extern "C" prevents C++ name mangling for reliable uprobe attachment.
// Does minimal work to maximize call frequency.
extern "C" __attribute__((noinline)) void target_func() {
    // A single volatile read prevents the compiler from optimizing this away
    // while keeping the function body as cheap as possible.
    static volatile int sink = 0;
    (void)sink;
}

// Distinct caller functions to produce diverse stacks.
// Each thread calls target_func from a unique call site.
__attribute__((noinline)) void Caller0() {
    while (gRunning.load(std::memory_order::relaxed)) {
        target_func();
    }
}

__attribute__((noinline)) void Caller1() {
    while (gRunning.load(std::memory_order::relaxed)) {
        target_func();
    }
}

__attribute__((noinline)) void Caller2() {
    while (gRunning.load(std::memory_order::relaxed)) {
        target_func();
    }
}

__attribute__((noinline)) void Caller3() {
    while (gRunning.load(std::memory_order::relaxed)) {
        target_func();
    }
}

using TCallerFn = void (*)();

static constexpr std::array<TCallerFn, 4> Callers = {
    Caller0,
    Caller1,
    Caller2,
    Caller3,
};

static void PinToCpu(int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
        // Continue even if pinning fails -- still generates load.
    }
}

static int GetNumCpus() {
    int n = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    if (n <= 0) {
        throw std::runtime_error("cannot retrieve number of CPUs");
    }
    return n > 0 ? n : 1;
}

int main(int argc, const char* argv[]) {
    int durationSec = 30;
    int concurrency = 0;

    NLastGetopt::TOpts opts;
    opts.AddLongOption("duration", "Duration in seconds")
        .RequiredArgument("N")
        .DefaultValue("30")
        .StoreResult(&durationSec);
    opts.AddLongOption("concurrency", "Number of worker threads (defaults to number of online CPUs)")
        .RequiredArgument("N")
        .DefaultValue("0")
        .StoreResult(&concurrency);
    NLastGetopt::TOptsParseResult(&opts, argc, argv);

    const int numCpus = GetNumCpus();
    const int numThreads = concurrency > 0 ? concurrency : numCpus;

    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int i = 0; i < numThreads; i++) {
        int cpu = i % numCpus;
        int index = i % static_cast<int>(Callers.size());
        threads.emplace_back([cpu, index]() {
            PinToCpu(cpu);
            Callers[index]();
        });
    }

    // Let the threads run for the specified duration.
    sleep(durationSec);
    gRunning.store(false, std::memory_order::relaxed);

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
