#include "../decode.h"

#include <llvm/MC/MCInst.h>

#include <util/stream/format.h>

#include <library/cpp/logger/log.h>
#include <library/cpp/logger/global/global.h>

#include <perforator/lib/asm/evaluator.h>
#include <perforator/lib/pthread/pthread.h>

namespace NPerforator::NPthread::NAsm {

std::expected<TAccessTSSInfo, EDecodePthreadGetspecificError> DecodePthreadGetspecific(
    [[maybe_unused]] const llvm::Triple& triple,
    [[maybe_unused]] TConstArrayRef<ui8> bytecode
) {
    return std::unexpected(EDecodePthreadGetspecificError::Unimplemented);
}

} // namespace NPerforator::NPthread::NAsm::NX86
