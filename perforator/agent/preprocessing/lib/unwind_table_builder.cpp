#include "unwind_table_builder.h"

#include "ehframe.h"
#include "sframe.h"

#include <util/generic/xrange.h>
#include <util/generic/yexception.h>

#include <perforator/lib/permutation/permutation.h>

namespace NPerforator::NBinaryProcessing::NUnwind {

void DifferentiateUnwindTable(NUnwind::UnwindTable& table) {
    // Delta-encode pc ranges
    ui64 pc = 0;
    for (int i : xrange(table.start_pc_size())) {
        ui64 start_pc = table.start_pc(i);
        ui64 pc_range = table.pc_range(i);

        Y_ENSURE(start_pc >= pc, "Mismatched pc: " << start_pc << " < " << pc);
        ui64 end = start_pc + pc_range;
        table.set_start_pc(i, start_pc - pc);
        pc = end;
    }
}

void IntegrateUnwindTable(NUnwind::UnwindTable& table) {
    ui64 pc = 0;
    for (int i : xrange(table.start_pc_size())) {
        table.set_start_pc(i, table.start_pc(i) + pc);
        pc = table.start_pc(i) + table.pc_range(i);
    }
}

void DeltaEncode(NUnwind::UnwindTable& table) {
    MultiSort(
        *table.mutable_start_pc(),
        *table.mutable_pc_range(),
        *table.mutable_cfa(),
        *table.mutable_rbp(),
        *table.mutable_ra());

    NUnwind::DifferentiateUnwindTable(table);
}

void RemapRules(google::protobuf::RepeatedField<ui32>* rules, const NUnwind::TRuleDict& dict) {
    for (auto& rule : *rules) {
        rule = dict.RemapRule(rule);
    }
}

UnwindTable BuildUnwindTable(llvm::object::ObjectFile* objectFile, const BinaryAnalysisOptions& opts) {
    UnwindTable ret;

    auto fillUnwindTable = [&ret, &opts, objectFile](UnwindInfoSource sourceType) -> void {
        switch (sourceType) {
            case UnwindInfoSource::Ehframe:
                ret = BuildUnwindTableFromEhFrame(objectFile, opts);
                break;
            case UnwindInfoSource::Sframe:
                ret = BuildUnwindTableFromSFrame(objectFile, opts);
                break;
            default:
                Y_ENSURE("Why we are here?");
                break;
        }
    };

    auto prefered = opts.GetPreferredUnwindInfoSource();
    auto fallback = opts.GetDefaultUnwindInfoSource();
    fillUnwindTable(prefered);
    if (ret.start_pc_size() == 0 && prefered != fallback) {
        fillUnwindTable(fallback);
    }

    return ret;
}

} // namespace NPerforator::NBinaryProcessing::NUnwind
