#include "lite_analysis.h"

#include <perforator/lib/elf/elf.h>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFObjectFile.h>

#include <util/generic/yexception.h>


namespace NPerforator::NLinguist::NJvm {

namespace {

// used during minimal analysis
constexpr static std::string_view kAbstractInterpreterCodeSym = "_ZN19AbstractInterpreter5_codeE";
constexpr static std::string_view kCodeCacheHeapsSym = "_ZN9CodeCache6_heapsE";

}

std::optional<TJvmAnalysis> ProcessJvmBinaryMinimal(const llvm::object::ObjectFile& binary) {
    TJvmAnalysis analysis;
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>* elfPtr = llvm::dyn_cast<llvm::object::ELFObjectFile<llvm::object::ELF64LE>>(&binary);
    Y_THROW_UNLESS(elfPtr != nullptr);
    const llvm::object::ELFObjectFile<llvm::object::ELF64LE>& elf = *elfPtr;
    NELF::TSymbolMap symbols = NELF::RetrieveSymbolsFromSymtabChecked(
        elf,
        kCodeCacheHeapsSym,
        kAbstractInterpreterCodeSym
    );
    if (symbols.empty()) {
        return std::nullopt;
    }
    analysis.Cheatsheet.set_code_cache_heaps(symbols.at(kCodeCacheHeapsSym).Address);
    analysis.Cheatsheet.set_abstract_interpreter_code(symbols.at(kAbstractInterpreterCodeSym).Address);
    return analysis;
}


} // namespace NPerforator::NLinguist::NJvm
