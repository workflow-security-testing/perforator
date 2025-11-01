#pragma once

#include <expected>

#include <llvm/Object/ObjectFile.h>

#include <util/generic/array_ref.h>

#include <perforator/lib/pthread/pthread.h>

namespace NPerforator::NPthread::NAsm {

enum class EDecodePthreadGetspecificError {
    FailedToDecodeInstructions,
    NoPthreadKeyDataValueOffset,
    NoPthreadKeyDataSize,
    NoPthreadKeysMax,
    NoPthreadKeyFirstLevelSize,
    NoPthreadKeySecondLevelSize,
    NoPthreadKeySpecificArrayOffset,
    NoPthreadKeySpecific1stBlockOffset,

    Unimplemented,
};

std::expected<TAccessTSSInfo, EDecodePthreadGetspecificError> DecodePthreadGetspecific(
    const llvm::Triple& triple,
    TConstArrayRef<ui8> bytecode
);

} // namespace NPerforator::NPthread::NAsm::NX86
