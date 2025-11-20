#include "tech_perforator_unwind_Native.h"

#include <perforator/lib/elf/elf.h>

#include <perforator/internal/linguist/jvm/unwind/lib/cheatsheet.h>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFObjectFile.h>

#include <util/stream/format.h>
#include <util/stream/output.h>

#include <util/system/yassert.h>
#include <util/system/compiler.h>

#include <dlfcn.h>
#include <link.h>

#include <string>
#include <span>

namespace {

struct TCodeHeapPtr {
    const void* P;
};

struct THeapBlockPtr {
    const void* P;
};

struct TCodeBlobPtr {
    const void* P;
};

struct TSymbolPtr {
    const void* P;
};

struct TNmethodPtr {
    const void* P;
};

struct TMethodPtr {
    const void* P;
};

struct TStubQueuePtr {
    const void* P;
};

struct TGrowableArrayPtr {
    const void* P;
};


class TShadow {
    NPerforator::NLinguist::NJvm::TJvmInfo Layout_;
private:
    size_t NextSegment(TCodeHeapPtr heap) {
        return *reinterpret_cast<const size_t*>(reinterpret_cast<const char*>(heap.P) + Layout_.CodeHeapNextSegmentFieldOffset);
    }

    size_t Log2SegmentSize(TCodeHeapPtr heap) {
        return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(heap.P) + Layout_.Analysis.CodeHeapLayout.Log2SegmentSizeFieldOffset);
    }

    const char* MemoryLow(TCodeHeapPtr heap) {
        const char* heapMemory = (reinterpret_cast<const char*>(heap.P) + Layout_.Analysis.CodeHeapLayout.MemoryFieldOffset);
        return *reinterpret_cast<char const* const*>(heapMemory + Layout_.Analysis.VirtualSpaceLayout.LowFieldOffset);
    }

    THeapBlockPtr BlockAt(TCodeHeapPtr heap, size_t i) {
        size_t segSizeLog2 = Log2SegmentSize(heap);

        return THeapBlockPtr{.P = MemoryLow(heap) + (i << segSizeLog2)};
    }

    size_t SegmentFor(TCodeHeapPtr heap, THeapBlockPtr block) {
        auto* ptr = reinterpret_cast<const char*>(block.P);
        return (ptr - MemoryLow(heap)) >> Log2SegmentSize(heap);
    }

    uint32_t BlockLength(THeapBlockPtr block) {
        return *reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(block.P) + Layout_.Analysis.HeapBlockLayout.HeaderFieldOffset + Layout_.Analysis.HeapBlockHeaderLayout.LengthFieldOffset);
    }

    bool BlockUsed(THeapBlockPtr block) {
        return *reinterpret_cast<const bool*>(reinterpret_cast<const char*>(block.P) + Layout_.Analysis.HeapBlockLayout.HeaderFieldOffset + Layout_.Analysis.HeapBlockHeaderLayout.UsedFieldOffset);
    }

public:
    TShadow(NPerforator::NLinguist::NJvm::TJvmInfo layout) : Layout_(layout) {}
    THeapBlockPtr FirstBlock(TCodeHeapPtr heap) {
        if (0 < NextSegment(heap)) {
            return BlockAt(heap, 0);
        }
        return THeapBlockPtr{.P = nullptr};
    }

    THeapBlockPtr NextBlock(TCodeHeapPtr heap, THeapBlockPtr block) {
        Y_ABORT_IF(block.P == nullptr);
        size_t i = SegmentFor(heap, block) + BlockLength(block);
        if (i < NextSegment(heap)) {
            return BlockAt(heap, i);
        }
        return THeapBlockPtr{.P = nullptr};
    }

    bool BlockFree(THeapBlockPtr block) {
        return !BlockUsed(block);
    }

    THeapBlockPtr NextUsed(TCodeHeapPtr heap, THeapBlockPtr block) {
        Y_ABORT_IF(block.P == nullptr);
        if (BlockFree(block)) {
            block = NextBlock(heap, block);
        }
        return block;
    }

    TCodeBlobPtr AllocatedSpace(THeapBlockPtr block) {
        return TCodeBlobPtr{.P = reinterpret_cast<const char*>(block.P) + Layout_.Analysis.HeapBlockLayout.AllocatedSpaceOffset};
    }

    uintptr_t CodeBegin(TCodeBlobPtr blob) {
        int offset = *reinterpret_cast<const int*>(reinterpret_cast<const char*>(blob.P) + Layout_.Analysis.CodeBlobLayout.CodeOffsetFieldOffset);
        uintptr_t res = reinterpret_cast<uintptr_t>(blob.P) + offset;
        return res;
    }

    uintptr_t CodeEnd(TCodeBlobPtr blob) {
        int offset = *reinterpret_cast<const int*>(reinterpret_cast<const char*>(blob.P) + Layout_.Analysis.CodeBlobLayout.DataOffsetFieldOffset);
        return reinterpret_cast<uintptr_t>(blob.P) + offset;
    }

    const char* CodeBlobName(TCodeBlobPtr blob) {
        return *reinterpret_cast<char const* const*>(reinterpret_cast<const char*>(blob.P) + Layout_.Analysis.CodeBlobLayout.NameFieldOffset);
    }

private:
    std::string StringifySymbol(TSymbolPtr symbol) {
        ui16 length = *reinterpret_cast<const ui16*>(reinterpret_cast<const char*>(symbol.P) + Layout_.Analysis.SymbolLayout.LengthFieldOffset);
        const ui8* bytes = reinterpret_cast<const ui8*>(symbol.P) + Layout_.Analysis.SymbolLayout.BodyFieldOffset;
        std::basic_string_view sv{bytes, length};
        std::string s;
        for (ui8 c : sv) {
            s.push_back(c);
        }
        return s;
    }

    const char* GetConstantFromPool(const char* constantPool, size_t index) {
        const char* addr = constantPool + Layout_.Analysis.ConstantPoolLayout.BaseOffset + index * sizeof(void*);
        return *reinterpret_cast<char const* const*>(addr);
    }

public:
    TMethodPtr GetMethod(TNmethodPtr nmethod) {
        return TMethodPtr{
            .P = *reinterpret_cast<void const* const*>(reinterpret_cast<const char*>(nmethod.P) + Layout_.Analysis.NmethodLayout.MethodFieldOffset)
        };
    }

    std::string MethodName(TMethodPtr method) {
        const char* constMethod = *reinterpret_cast<char const* const*>(reinterpret_cast<const char*>(method.P) + Layout_.Analysis.MethodLayout.ConstMethodFieldOffset);
        const char* constantPool = *reinterpret_cast<char const* const*>(constMethod + Layout_.Analysis.ConstMethodLayout.ConstantsFieldOffset);
        ui16 nameIndex = *reinterpret_cast<ui16 const*>(constMethod + Layout_.Analysis.ConstMethodLayout.NameIndexFieldOffset);
        Cerr << "Method name index: " << nameIndex << Endl;
        const char* nameSym = GetConstantFromPool(constantPool, nameIndex);
        Cerr << "Method name symbol: " << SHex(reinterpret_cast<uintptr_t>(nameSym)) << Endl;
        std::string selfName = StringifySymbol(TSymbolPtr{.P = nameSym});
        const char* poolHolder = *reinterpret_cast<char const* const*>(constantPool + Layout_.Analysis.ConstantPoolLayout.PoolHolderFieldOffset);
        const char* klassNameSym = *reinterpret_cast<char const* const*>(poolHolder + Layout_.Analysis.KlassLayout.NameFieldOffset);
        Cerr << "Class name symbol: " << SHex(reinterpret_cast<uintptr_t>(klassNameSym)) << Endl;
        std::string klassName = StringifySymbol(TSymbolPtr{.P = klassNameSym});
        return klassName + '.' + selfName;
    }

    uintptr_t StubQueueCodeBegin(TStubQueuePtr queue) {
        return *reinterpret_cast<uintptr_t const*>(reinterpret_cast<const char*>(queue.P) + Layout_.Analysis.StubQueueLayout.StubBufferFieldOffset);
    }

    size_t StubQueueCodeSize(TStubQueuePtr queue) {
        return *reinterpret_cast<const int*>(reinterpret_cast<const char*>(queue.P) + Layout_.Analysis.StubQueueLayout.BufferLimitFieldOffset);
    }

    template<typename T>
    std::span<const T> ArrayData(TGrowableArrayPtr array) {
        const T* data = *reinterpret_cast<T const* const*>(reinterpret_cast<const char*>(array.P) + Layout_.Analysis.GrowableArrayLayout.DataFieldOffset);
        size_t size = *reinterpret_cast<const int*>(reinterpret_cast<const char*>(array.P) + Layout_.Analysis.GrowableArrayLayout.LenFieldOffset);
        return std::span<const T>(data, size);
    }

    size_t ReturnAddressOffset() {
        return Layout_.StackFrameLayout.ReturnAddressOffset;
    }

    size_t InterpreterFrameMethodOffset() {
        return Layout_.StackFrameLayout.InterpreterFrameMethodOffset;
    }
};

struct TMethodInfo {
    uintptr_t Begin;
    uintptr_t End;
    std::string Desc;
    std::string MethodName;
    bool Compiled = false;
};

struct TJvmInfo {
    uintptr_t InterpreterBegin;
    uintptr_t InterpreterEnd;

    std::vector<TMethodInfo> Methods;

    TShadow Shadow;

    TJvmInfo(TShadow shadow) : Shadow(shadow) {}

    struct TLibJvmInfo {
        std::string Path;
        uintptr_t Base;
    };

    static TJvmInfo Resolve() {
        TLibJvmInfo libjvm;
        dl_iterate_phdr([](dl_phdr_info* info, size_t, void* data) {
            std::string_view path = info->dlpi_name;
            if (path.contains("libjvm.so")) {
                TLibJvmInfo& p = *reinterpret_cast<TLibJvmInfo*>(data);
                Y_ABORT_UNLESS(p.Path == "");
                p.Path = path;
                p.Base = info->dlpi_addr;
            }
            return 0;
        }, &libjvm);
        Y_ABORT_UNLESS(libjvm.Path != "");
        Cout << "Found libjvm.so at " << libjvm.Path << Endl;
        llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>> f = llvm::object::ObjectFile::createObjectFile(libjvm.Path);
        Y_ABORT_UNLESS(f);
        llvm::object::ELFObjectFile<llvm::object::ELF64LE>* elf = llvm::dyn_cast<llvm::object::ELFObjectFile<llvm::object::ELF64LE>>(f->getBinary());
        Y_ABORT_UNLESS(elf != nullptr);
        TMaybe<THashMap<TStringBuf, NPerforator::NELF::TLocation>> symbols = NPerforator::NELF::RetrieveSymbolsFromSymtab(
            *elf,
            NPerforator::NLinguist::NJvm::TVMStructsAddresses::StructsAddressSym,
            //NPerforator::NLinguist::NJvm::TVMStructsAddresses::StructsLengthSym,
            NPerforator::NLinguist::NJvm::TVMStructsAddresses::TypesAddressSym//,
            //NPerforator::NLinguist::NJvm::TVMStructsAddresses::TypesLengthSym
        );
        Y_ABORT_UNLESS(symbols.Defined());
        Y_ABORT_UNLESS(symbols->size() == 2);
        NPerforator::NLinguist::NJvm::TVMStructsAddresses addresses;
        auto getOffset = [&symbols, &libjvm](const TStringBuf& name) {
            uintptr_t address = libjvm.Base + symbols->at(name).Address;
            Cout << "Found symbol: " << name << " at " << SHex(address) << Endl;
            return reinterpret_cast<void*>(address);
        };
        addresses.StructsAddress = getOffset(NPerforator::NLinguist::NJvm::TVMStructsAddresses::StructsAddressSym);
        addresses.TypesAddress = getOffset(NPerforator::NLinguist::NJvm::TVMStructsAddresses::TypesAddressSym);

        NPerforator::NLinguist::NJvm::TJvmInfo access = NPerforator::NLinguist::NJvm::GetFromVMStructs(addresses);
        Cout << "Interpteter code pointer address: " << SHex(access.Analysis.AbstractInterpreterCodeAddress) << Endl;

        TShadow shadow{access};


        const void* heapsAddress = *reinterpret_cast<void const* const*>(access.Analysis.CodeCacheHeapsAddress);
        std::span<const TCodeHeapPtr> heaps = shadow.ArrayData<TCodeHeapPtr>(TGrowableArrayPtr{.P = heapsAddress});
        std::vector<TMethodInfo> methods;
        {
            Cout << "Parsing code heap" << Endl;
            for (size_t i = 0; i < heaps.size(); ++i) {
                TCodeHeapPtr heap = heaps[i];
                Cout << "Heap " << i << Endl;
                THeapBlockPtr block = shadow.FirstBlock(heap);
                int nonNmethods = 0;
                size_t blocks = 0;
                size_t freeBlocks = 0;
                while (block.P != nullptr) {
                    if (shadow.BlockFree(block)) {
                        ++freeBlocks;
                        block = shadow.NextUsed(heap, block);
                    }
                    if (block.P == nullptr) {
                        break;
                    }
                    Y_ABORT_IF(shadow.BlockFree(block));
                    TCodeBlobPtr cb = shadow.AllocatedSpace(block);
                    TMethodInfo minfo;
                    minfo.Begin = shadow.CodeBegin(cb);
                    minfo.End = shadow.CodeEnd(cb);
                    ++blocks;

                    auto cbKind = *reinterpret_cast<const unsigned char*>(reinterpret_cast<const char*>(cb.P) + access.KindFieldOffset);

                    if (cbKind == access.NmethodKind) {
                        minfo.Desc = "nmethod";
                        TMethodPtr method = shadow.GetMethod(TNmethodPtr{.P = cb.P});
                        minfo.Compiled = true;
                        minfo.MethodName = shadow.MethodName(method);
                    } else {
                        minfo.Desc = "not nmethod";
                        ++nonNmethods;
                    }
                    char const* cbName = shadow.CodeBlobName(cb);

                    minfo.Desc += "; name=";
                    if (cbName != nullptr) {
                        minfo.Desc += cbName;
                    } else {
                        minfo.Desc += "<nullptr>";
                    }

                    methods.push_back(minfo);
                    block = shadow.NextBlock(heap, block);
                }
                Cout << "Found " << blocks << " used blocks, non-nmethods: " << nonNmethods << ", free: " << freeBlocks << Endl;
            }
            Cout << "Code heap parsed!" << Endl;
        }
        TJvmInfo result{shadow};
        TStubQueuePtr queue{.P = *reinterpret_cast<void**>(access.Analysis.AbstractInterpreterCodeAddress)};
        result.InterpreterBegin = shadow.StubQueueCodeBegin(queue);
        result.InterpreterEnd = result.InterpreterBegin + shadow.StubQueueCodeSize(queue);
        result.Methods = std::move(methods);

        return result;
    }
};

struct TUnwinder {
    TJvmInfo Info;

    void ProcessFrame(uintptr_t rbp, uintptr_t ip) {
        Cout << "-----> Frame: base=" << SHex(rbp) << ", ip=" << SHex(ip) << Endl;
        if (Info.InterpreterBegin <= ip && ip < Info.InterpreterEnd) {
            ProcessInterpretedFrame(rbp, ip);
            return;
        }
        bool matched = false;
        for (auto const& method : Info.Methods) {
            if (method.Begin > ip || ip >= method.End) {
                continue;
            }
            Y_ABORT_IF(matched);
            matched = true;
            if (method.Compiled) {
                ProcessCompiledFrame(rbp, ip, method);
            } else {
                ProcessGeneratedFrame(rbp, ip, method);
            }
            return;
        }
        Cout << "TODO: unsupported frame" << Endl;
    }

    void ProcessCompiledFrame(uintptr_t rbp, uintptr_t ip, const TMethodInfo& method) {
        Y_UNUSED(ip); // TODO - resolve line
        Cout << "Kind: compiled" << Endl;
        Cout << "Name: " << method.MethodName << Endl;
        Cout << "Description: " << method.Desc << Endl;
        uintptr_t callerIp = *reinterpret_cast<uintptr_t*>(rbp + Info.Shadow.ReturnAddressOffset());
        uintptr_t prevFrame = *reinterpret_cast<uintptr_t*>(rbp);
        ProcessFrame(prevFrame, callerIp);
    }

    void ProcessGeneratedFrame(uintptr_t rbp, uintptr_t ip, const TMethodInfo& method) {
        Y_UNUSED(ip); // TODO - resolve line
        Cout << "Kind: generated" << Endl;
        Cout << "Description: " << method.Desc << Endl;
        uintptr_t callerIp = *reinterpret_cast<uintptr_t*>(rbp + Info.Shadow.ReturnAddressOffset());
        uintptr_t prevFrame = *reinterpret_cast<uintptr_t*>(rbp);
        ProcessFrame(prevFrame, callerIp);
    }

    void ProcessInterpretedFrame(uintptr_t rbp, uintptr_t ip) {
        Y_UNUSED(ip); // TODO - resolve line
        Cout << "Kind: interpreted" << Endl;

        Cerr << "Return address located at: " << SHex(rbp + Info.Shadow.ReturnAddressOffset()) << Endl;
        uintptr_t callerIp = *reinterpret_cast<uintptr_t*>(rbp + Info.Shadow.ReturnAddressOffset());
        Cerr << "Return address: " << SHex(callerIp) << Endl;
        const void* method;
        Cerr << "Method pointer located at: " << SHex(rbp + Info.Shadow.InterpreterFrameMethodOffset()) << Endl;
        method = *reinterpret_cast<void const* const*>(rbp + Info.Shadow.InterpreterFrameMethodOffset());
        Cerr << "Method pointer: " << SHex(reinterpret_cast<uintptr_t>(method)) << Endl;
        Cout << "Name: " << Info.Shadow.MethodName(TMethodPtr{.P = method}) << Endl;

        Cerr << "Old frame pointer located at: " << SHex(rbp) << Endl;
        uintptr_t prevFrame = *reinterpret_cast<uintptr_t*>(rbp);
        Cerr << "Restored frame pointer: " << SHex(prevFrame) << Endl;
        ProcessFrame(prevFrame, callerIp);
    }
};

void Unwind(TUnwinder& unwinder, uintptr_t callerFrameStart, uintptr_t callerIp) {
    Cout << "\n\n\n>>>>>> Unwinding started" << Endl;
    unwinder.ProcessFrame(callerFrameStart, callerIp);
    Cout << "<<<<<< Unwinding finished" << Endl;
}
}

extern "C" {

JNIEXPORT jlong JNICALL Java_tech_perforator_unwind_Native_make0(JNIEnv* env, jclass cls) {
    Y_UNUSED(env, cls);
    TJvmInfo jvmInfo = TJvmInfo::Resolve();
    Cout << "Interpreter: [" << SHex(jvmInfo.InterpreterBegin) << ", " << SHex(jvmInfo.InterpreterEnd) << ")" << Endl;

    auto u = new TUnwinder{jvmInfo};
    return reinterpret_cast<jlong>(u);
}

JNIEXPORT void JNICALL Java_tech_perforator_unwind_Native_unwind0(JNIEnv* env, jclass cls, jlong impl) {
    Y_UNUSED(env, cls);
    uintptr_t rbp = 0;
    __asm__("mov %%rbp, %0" : "=r"(rbp));
    uintptr_t callerFrameStart;
    uintptr_t callerIp;
    std::memcpy(&callerFrameStart, reinterpret_cast<void*>(rbp), sizeof(callerFrameStart));
    std::memcpy(&callerIp, reinterpret_cast<void*>(rbp + sizeof(void*)), sizeof(callerIp));
    Unwind(*reinterpret_cast<TUnwinder*>(impl), callerFrameStart, callerIp);
}

JNIEXPORT void JNICALL Java_tech_perforator_unwind_Native_unwindIfZero0(JNIEnv* env, jclass cls, jlong impl, jint x) {
    Y_UNUSED(env, cls);
    uintptr_t rbp = 0;
    __asm__("mov %%rbp, %0" : "=r"(rbp));
    uintptr_t callerFrameStart;
    uintptr_t callerIp;
    std::memcpy(&callerFrameStart, reinterpret_cast<void*>(rbp), sizeof(callerFrameStart));
    std::memcpy(&callerIp, reinterpret_cast<void*>(rbp + sizeof(void*)), sizeof(callerIp));
    if (x == 0) {
        Unwind(*reinterpret_cast<TUnwinder*>(impl), callerFrameStart, callerIp);
    }
}

}
