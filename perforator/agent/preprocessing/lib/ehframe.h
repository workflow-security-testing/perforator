#pragma once

#include "unwind_table_builder.h"

namespace NPerforator::NBinaryProcessing::NUnwind {

    UnwindTable BuildUnwindTableFromEhFrame(llvm::object::ObjectFile* objectFile, const BinaryAnalysisOptions& opts);

} // namespace NPerforator::NBinaryProcessing::NUnwind
