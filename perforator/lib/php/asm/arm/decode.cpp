#include "decode.h"

namespace NPerforator::NLinguist::NPhp::NAsm::NArm {

TMaybe<ui64> DecodePhpVersion(
    const llvm::Triple& /*triple*/,
    ui64 /*functionAddress*/,
    TConstArrayRef<ui8> /*bytecode*/
) {
    // Not supported yet
    return Nothing();
}

TMaybe<ui64> DecodeZmInfoPhpCore(
    const llvm::Triple& /*triple*/,
    ui64 /*functionAddress*/,
    TConstArrayRef<ui8> /*bytecode*/
) {
    // Not supported yet
    return Nothing();
}

TMaybe<ui64> DecodeZendVmKind(
    const llvm::Triple& /*triple*/,
    ui64 /*functionAddress*/,
    TConstArrayRef<ui8> /*bytecode*/
) {
    // Not supported yet
    return Nothing();
}

} // namespace NPerforator::NLinguist::NPhp::NAsm::NArm
