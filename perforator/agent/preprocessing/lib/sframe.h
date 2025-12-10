#pragma once

#include "unwind_table_builder.h"

namespace NPerforator::NBinaryProcessing::NUnwind {

    UnwindTable BuildUnwindTableFromSFrame(llvm::object::ObjectFile* objectFile, const BinaryAnalysisOptions& opts);

} // namespace NPerforator::NBinaryProcessing::NUnwind
