#pragma once

#include "output.h"

#include <llvm/Object/ObjectFile.h>

#include <optional>

namespace NPerforator::NLinguist::NJvm {

// ProcessJvmBinaryMinimal creates a partial analysis (i.e. it does not parse
// JVM symbols with type and field information).
// It is expected that caller already has rest of the information.
// Note also that ProcessJvmBinaryNormal is effectively not implemented yet.

std::optional<TJvmAnalysis> ProcessJvmBinaryNormal(const llvm::object::ObjectFile& binary);
std::optional<TJvmAnalysis> ProcessJvmBinaryMinimal(const llvm::object::ObjectFile& binary);

}
