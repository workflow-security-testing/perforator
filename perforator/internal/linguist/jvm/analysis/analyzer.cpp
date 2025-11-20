#include "analyzer.h"

#include "offset_registry.h"
#include "analyzer_impl.h"

#include <perforator/lib/elf/elf.h>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFObjectFile.h>

#include <util/generic/yexception.h>

#include <span>

namespace NPerforator::NLinguist::NJvm {

namespace {

// used during normal analysis
constexpr static std::string_view kStructsAddressSym = "_ZN9VMStructs21localHotSpotVMStructsE";
constexpr static std::string_view kTypesAddressSym = "_ZN9VMStructs19localHotSpotVMTypesE";

// used during minimal analysis
constexpr static std::string_view kAbstractInterpreterCodeSym = "_ZN19AbstractInterpreter5_codeE";
constexpr static std::string_view kCodeCacheHeapsSym = "_ZN9CodeCache6_heapsE";

THashMap<TStringBuf, NELF::TLocation> RetrieveSymbolsFromSymtabChecked(
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>& elf,
    std::same_as<std::string_view> auto ...symbols
) {
    TMaybe<THashMap<TStringBuf, NPerforator::NELF::TLocation>> res = NELF::RetrieveSymbolsFromSymtab(elf, symbols...);
    if (res.Empty()) {
        throw yexception() << "Unknown ELF kind";
    }
    if (res->size() != sizeof...(symbols) && res->size() != 0) {
        throw yexception() << "Found only subset of expected symbols";
    }
    return std::move(*res);
}
}

std::optional<TJvmAnalysis> ProcessJvmBinaryMinimal(const llvm::object::ObjectFile& binary) {
    TJvmAnalysis analysis;
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>* elfPtr = llvm::dyn_cast<llvm::object::ELFObjectFile<llvm::object::ELF64LE>>(&binary);
    Y_THROW_UNLESS(elfPtr != nullptr);
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>& elf = *elfPtr;
    THashMap<TStringBuf, NPerforator::NELF::TLocation> symbols = RetrieveSymbolsFromSymtabChecked(
        elf,
        kCodeCacheHeapsSym,
        kAbstractInterpreterCodeSym
    );
    if (symbols.empty()) {
        return std::nullopt;
    }
    analysis.CodeCacheHeapsAddress = symbols.at(kCodeCacheHeapsSym).Address;
    analysis.AbstractInterpreterCodeAddress = symbols.at(kAbstractInterpreterCodeSym).Address;
    return analysis;
}


std::optional<TJvmAnalysis> ProcessJvmBinaryNormal(const llvm::object::ObjectFile& binary) {
    TJvmAnalysis offsets;
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>* elfPtr = llvm::dyn_cast<llvm::object::ELFObjectFile<llvm::object::ELF64LE>>(&binary);
    Y_THROW_UNLESS(elfPtr != nullptr);
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>& elf = *elfPtr;
    THashMap<TStringBuf, NPerforator::NELF::TLocation> symbols = RetrieveSymbolsFromSymtabChecked(
        elf,
        kStructsAddressSym,
        kTypesAddressSym
    );
    if (symbols.empty()) {
        return std::nullopt;
    }
    auto getSymbol = [&symbols, &elf](const TStringBuf& name) -> TArrayRef<const unsigned char> {
        NPerforator::NELF::TLocation sym = symbols.at(name);
        auto content = NPerforator::NELF::RetrieveContentFromRodataSection(elf, sym);
        Y_THROW_UNLESS(content.Defined());
        Y_THROW_UNLESS(content->size() == sym.Size);
        return *content;
    };
    TArrayRef<const unsigned char> structsSym = getSymbol(kStructsAddressSym);
    TArrayRef<const unsigned char> typesSym = getSymbol(kTypesAddressSym);

    if (structsSym.size() % sizeof(THotSpotStructEntry) != 0) {
        throw yexception() << "Invalid structs length";
    }
    if (structsSym.size() / sizeof(THotSpotStructEntry) < 10) {
        throw yexception() << "Suspiciously short structs list";
    }

    if (typesSym.size() % sizeof(THotSpotTypeEntry) != 0) {
        throw yexception() << "Invalid types length";
    }
    if (typesSym.size() / sizeof(THotSpotTypeEntry) < 10) {
        throw yexception() << "Suspiciously short types list";
    }

    std::vector<THotSpotStructEntry> structs;
    std::vector<THotSpotTypeEntry> types;

    structs.resize(structsSym.size() / sizeof(THotSpotStructEntry));
    types.resize(typesSym.size() / sizeof(THotSpotTypeEntry));

    // we could use std::start_lifetime_as_array instead of this memcpy, but as of 2025 it does not seem
    // to be implemented anywhere :(
    std::memcpy(structs.data(), structsSym.data(), structsSym.size());
    std::memcpy(types.data(), typesSym.data(), typesSym.size());

    TJvmMetadata metadata{
        std::span<const THotSpotStructEntry>{
            structs.data(),
            structs.size(),
        },
        std::span<const THotSpotTypeEntry>{
            types.data(),
            typesSym.size(),
        }
    };

    return ProcessOffsetRegistry(metadata);
}

} // namespace NPerforator::NLinguist::NJvm
