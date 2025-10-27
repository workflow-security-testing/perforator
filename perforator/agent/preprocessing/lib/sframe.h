#pragma once

#include <llvm/Object/ObjectFile.h>

struct TFunctionDescriptionEntry {
    int32_t pc;
    uint32_t size;
};

struct TFunctionRowEntry {
    uint8_t info;
    uint32_t pc;
    uint32_t range;
    llvm::SmallVector<int32_t> offsets;
};

struct TFdeFre {
    TFunctionDescriptionEntry fde;
    TFunctionRowEntry fre;
};

using TFreHandler = std::function<void(const TFdeFre&)>;

void IterateOverSframeFre(llvm::object::ObjectFile* objectFile, TFreHandler handle);
