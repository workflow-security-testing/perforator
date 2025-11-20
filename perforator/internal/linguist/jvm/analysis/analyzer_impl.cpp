#include "analyzer_impl.h"

namespace NPerforator::NLinguist::NJvm {

TJvmAnalysis ProcessOffsetRegistry(const TJvmMetadata& metadata) {
    TJvmAnalysis offsets;
    offsets.CodeHeapLayout.MemoryFieldOffset = metadata.FindFieldOffset("CodeHeap", "_memory");
    offsets.CodeHeapLayout.Log2SegmentSizeFieldOffset = metadata.FindFieldOffset("CodeHeap", "_log2_segment_size");
    offsets.CodeHeapLayout.SegmapFieldOffset = metadata.FindFieldOffset("CodeHeap", "_segmap");

    offsets.VirtualSpaceLayout.LowFieldOffset = metadata.FindFieldOffset("VirtualSpace", "_low");
    offsets.VirtualSpaceLayout.HighFieldOffset = metadata.FindFieldOffset("VirtualSpace", "_high");

    offsets.CodeBlobLayout.CodeOffsetFieldOffset = metadata.FindFieldOffset("CodeBlob", "_code_offset");
    offsets.CodeBlobLayout.DataOffsetFieldOffset = metadata.FindFieldOffset("CodeBlob", "_data_offset");
    offsets.CodeBlobLayout.NameFieldOffset = metadata.FindFieldOffset("CodeBlob", "_name");

    offsets.HeapBlockLayout.HeaderFieldOffset = metadata.FindFieldOffset("HeapBlock", "_header");
    offsets.HeapBlockHeaderLayout.LengthFieldOffset = metadata.FindFieldOffset("HeapBlock::Header", "_length");
    offsets.HeapBlockHeaderLayout.UsedFieldOffset = metadata.FindFieldOffset("HeapBlock::Header", "_used");
    offsets.HeapBlockLayout.AllocatedSpaceOffset = metadata.FindTypeSize("HeapBlock");

    offsets.NmethodLayout.MethodFieldOffset = metadata.FindFieldOffset("nmethod", "_method");

    offsets.MethodLayout.ConstMethodFieldOffset = metadata.FindFieldOffset("Method", "_constMethod");

    offsets.ConstMethodLayout.ConstantsFieldOffset = metadata.FindFieldOffset("ConstMethod", "_constants");
    offsets.ConstMethodLayout.NameIndexFieldOffset = metadata.FindFieldOffset("ConstMethod", "_name_index");

    offsets.ConstantPoolLayout.BaseOffset = metadata.FindTypeSize("ConstantPool");
    offsets.ConstantPoolLayout.PoolHolderFieldOffset = metadata.FindFieldOffset("ConstantPool", "_pool_holder");

    offsets.KlassLayout.NameFieldOffset = metadata.FindFieldOffset("Klass", "_name");

    offsets.SymbolLayout.BodyFieldOffset = metadata.FindFieldOffset("Symbol", "_body[0]");
    offsets.SymbolLayout.LengthFieldOffset = metadata.FindFieldOffset("Symbol", "_length");

    offsets.StubQueueLayout.StubBufferFieldOffset = metadata.FindFieldOffset("StubQueue", "_stub_buffer");
    offsets.StubQueueLayout.BufferLimitFieldOffset = metadata.FindFieldOffset("StubQueue", "_buffer_limit");

    offsets.GrowableArrayLayout.LenFieldOffset = metadata.FindFieldOffset("GrowableArrayBase", "_len");
    offsets.GrowableArrayLayout.DataFieldOffset = metadata.FindFieldOffset("GrowableArray<int>", "_data");

    offsets.CodeCacheHeapsAddress = reinterpret_cast<uint64_t>(metadata.FindStaticFieldAddress("CodeCache", "_heaps"));
    offsets.AbstractInterpreterCodeAddress = reinterpret_cast<uint64_t>(metadata.FindStaticFieldAddress("AbstractInterpreter", "_code"));

    return offsets;
}

}
