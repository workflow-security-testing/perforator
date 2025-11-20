#pragma once

#include <util/system/types.h>

#include <span>
#include <string_view>

namespace NPerforator::NLinguist::NJvm {

struct THotSpotStructEntry {
    const char* StructName;
    const char* FieldName;
    const char* TypeName;
    ui64 IsStatic;
    ui64 Offset;
    void* Address;
};

struct THotSpotTypeEntry {
    const char* StructName;
    const char* SuperName;
    i32 IsOop;
    i32 IsInteger;
    i32 IsUnsigned;
    ui64 Size;
};

class TJvmMetadata {
private:
    std::span<const THotSpotStructEntry> Structs_;
    std::span<const THotSpotTypeEntry> Types_;

public:
    TJvmMetadata(std::span<const THotSpotStructEntry> structsSym, std::span<const THotSpotTypeEntry> typesSym);

private:
    const THotSpotStructEntry* FindField(std::string_view typeName, std::string_view fieldName) const;

public:
    void* FindStaticFieldAddress(std::string_view typeName, std::string_view fieldName) const;

    size_t FindFieldOffset(std::string_view typeName, std::string_view fieldName) const;

    size_t FindTypeSize(std::string_view typeName) const;
};

}
