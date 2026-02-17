#include "static_analysis.h"

#include "offsets.h"

#include <perforator/internal/linguist/jvm/analysis/offset_registry/analyzer_impl.h>
#include <perforator/internal/linguist/jvm/analysis/offset_registry/offset_registry.h>

#include <util/stream/output.h>
#include <util/system/yassert.h>

#include <span>


namespace NPerforator::NLinguist::NJvm {

namespace {

size_t StructsLength(const THotSpotStructEntry* entries) {
    size_t length = 0;
    while (entries[length].StructName != nullptr || entries[length].FieldName != nullptr) {
        ++length;
    }
    return length;
}

size_t TypesLength(const THotSpotTypeEntry* entries) {
    size_t length = 0;
    while (entries[length].StructName != nullptr) {
        ++length;
    }
    return length;
}

};

TJvmAnalysis ProcessJVMHeaders() {
    TJvmAnalysis analysis;

    TOffsets offsets = TOffsets::Get();

    analysis.Cheatsheet.set_code_blob_kind(offsets.CodeBlobKindOffset);
    analysis.Cheatsheet.set_code_blob_kind_nmethod(static_cast<int>(offsets.CodeBlobKindNmethod));
    analysis.Cheatsheet.set_code_heap_next_segment(offsets.CodeHeapNextSegmentOffset);
    analysis.Cheatsheet.set_frame_return_addr_offset(offsets.StackFrameReturnAddressOffset);
    analysis.Cheatsheet.set_frame_interpreter_frame_method_offset(offsets.InterpreterStackFrameMethodOffset);

    return analysis;
}

TJvmAnalysis ProcessDynamicLinkedJVM(TVMStructsAddresses addresses) {
    auto* structs = *reinterpret_cast<const THotSpotStructEntry**>(addresses.StructsAddress);
    auto* types = *reinterpret_cast<const THotSpotTypeEntry**>(addresses.TypesAddress);
    size_t structsLength = StructsLength(structs);
    size_t typesLength = TypesLength(types);
    TJvmMetadata metadata{
        std::span<const THotSpotStructEntry>(structs, structsLength),
        std::span<const THotSpotTypeEntry>(types, typesLength)
    };
    TJvmAnalysis analysis = ProcessOffsetRegistry(metadata, TOffsetRegistryAnalysisOptions{});
    return analysis;
}

} // namespace NPerforator::NLinguist::NJvm
