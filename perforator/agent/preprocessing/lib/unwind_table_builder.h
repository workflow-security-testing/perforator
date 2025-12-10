#pragma once

#include <llvm/Object/ObjectFile.h>
#include <perforator/agent/preprocessing/proto/parse/parse.pb.h>
#include <perforator/agent/preprocessing/proto/unwind/table.pb.h>

#include "rule_dict.h"

namespace NPerforator::NBinaryProcessing::NUnwind {

    void DifferentiateUnwindTable(UnwindTable& table);
    void IntegrateUnwindTable(UnwindTable& table);
    void DeltaEncode(UnwindTable& table);
    void RemapRules(google::protobuf::RepeatedField<ui32>* rules, const TRuleDict& dict);

    UnwindTable BuildUnwindTable(llvm::object::ObjectFile* objectFile, const BinaryAnalysisOptions& opts);

} // namespace NPerforator::NBinaryProcessing::NUnwind
