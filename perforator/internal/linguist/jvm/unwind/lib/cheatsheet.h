#include <string>
#include <cstddef>

#include <perforator/internal/linguist/jvm/analysis/output.h>

namespace NPerforator::NLinguist::NJvm {


// Information extracted from the VMStructs class.
// See https://github.com/openjdk/jdk/blob/89f9268ed7c2cb86891f23a10482cd459454bd32/src/hotspot/share/runtime/vmStructs.hpp#L34
struct TVMStructsAddresses {
    constexpr static std::string_view StructsAddressSym = "gHotSpotVMStructs";
    void* StructsAddress;
    //constexpr static std::string_view StructsLengthSym = "_ZN9VMStructs27localHotSpotVMStructsLengthEv";
    //void* StructsLength;
    constexpr static std::string_view TypesAddressSym = "gHotSpotVMTypes";
    void* TypesAddress;
    //constexpr static std::string_view TypesLengthSym = "_ZN9VMStructs25localHotSpotVMTypesLengthEv";
    //void* TypesLength;
};

struct TStackFrameLayout {
    ssize_t ReturnAddressOffset = SSIZE_MAX;
    ssize_t InterpreterFrameMethodOffset = SSIZE_MAX;
};

struct TJvmInfo {
    TJvmAnalysis Analysis;
    std::string VersionString;
    TStackFrameLayout StackFrameLayout;
    size_t KindFieldOffset = SIZE_MAX;
    unsigned char NmethodKind = UCHAR_MAX;
    size_t CodeHeapNextSegmentFieldOffset = SIZE_MAX;
};

TJvmInfo GetFromVMStructs(TVMStructsAddresses);


} // namespace NPerforator::NLinguist::NJvm
