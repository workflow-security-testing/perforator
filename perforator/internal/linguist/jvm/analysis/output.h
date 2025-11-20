#pragma once

#include <string>
#include <cstdint>
#include <cstdlib>

namespace NPerforator::NLinguist::NJvm {

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/memory/virtualspace.hpp#L34
struct TVirtualSpaceLayout {
    size_t LowFieldOffset = SIZE_MAX;
    size_t HighFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/memory/heap.hpp#L86
struct TCodeHeapLayout {
    size_t MemoryFieldOffset = SIZE_MAX;
    size_t Log2SegmentSizeFieldOffset = SIZE_MAX;
    size_t SegmapFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/code/codeBlob.hpp#L52
struct TCodeBlobLayout {
    size_t CodeOffsetFieldOffset = SIZE_MAX;
    size_t DataOffsetFieldOffset = SIZE_MAX;
    size_t NameFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/memory/heap.hpp#L38
struct THeapBlockLayout {
    size_t HeaderFieldOffset = SIZE_MAX;
    size_t AllocatedSpaceOffset = SIZE_MAX;
};

//https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/memory/heap.hpp#L42
struct THeapBlockHeaderLayout {
    size_t LengthFieldOffset = SIZE_MAX;
    size_t UsedFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/code/nmethod.hpp#L134
struct TNmethodLayout {
    size_t MethodFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/oops/method.hpp#L45
struct TMethodLayout {
    size_t ConstMethodFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/oops/constMethod.hpp#L33
struct TConstMethodLayout {
    size_t ConstantsFieldOffset = SIZE_MAX;
    size_t NameIndexFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/oops/constantPool.hpp#L43
struct TConstantPoolLayout {
    size_t BaseOffset = SIZE_MAX;
    size_t PoolHolderFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/oops/klass.hpp#L63
struct TKlassLayout {
    size_t NameFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/oops/symbol.hpp#L33
struct TSymbolLayout {
    size_t BodyFieldOffset = SIZE_MAX;
    size_t LengthFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/code/stubs.hpp#L146
struct TStubQueueLayout {
    size_t StubBufferFieldOffset = SIZE_MAX;
    size_t BufferLimitFieldOffset = SIZE_MAX;
};

// https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/utilities/growableArray.hpp#L692
// (also see above that line for base classes)
struct TGrowableArrayLayout {
    size_t LenFieldOffset = SIZE_MAX;
    size_t DataFieldOffset = SIZE_MAX;
};

struct TJvmAnalysis {
    // following two fields are addresses of libjvm.so symbols necessary for unwinding and symbolization
    // https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/interpreter/abstractInterpreter.hpp#L37
    uint64_t AbstractInterpreterCodeAddress;
    // https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/code/codeCache.hpp#L38
    uint64_t CodeCacheHeapsAddress;

    TCodeHeapLayout CodeHeapLayout;
    TVirtualSpaceLayout VirtualSpaceLayout;
    TCodeBlobLayout CodeBlobLayout;
    THeapBlockLayout HeapBlockLayout;
    THeapBlockHeaderLayout HeapBlockHeaderLayout;

    TNmethodLayout NmethodLayout;
    TMethodLayout MethodLayout;
    TConstMethodLayout ConstMethodLayout;
    TConstantPoolLayout ConstantPoolLayout;
    TKlassLayout KlassLayout;
    TSymbolLayout SymbolLayout;

    TStubQueueLayout StubQueueLayout;
    TGrowableArrayLayout GrowableArrayLayout;
};

}
