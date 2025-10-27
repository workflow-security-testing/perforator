#include "sframe.h"

#include <perforator/lib/llvmex/llvm_exception.h>
#include <llvm/ADT/StringExtras.h>

#if LLVM_VERSION_MAJOR < 22
    #include "llvm/SFrameParser.h"
#else
    #error "You should use <llvm/Object/SFrameParser.h>"
#endif

namespace {
    template <llvm::endianness ESelectedEndian>
    void iterateFdeImpl(llvm::object::SectionRef sframeSection, TFreHandler handle) {
        using TParser = llvm::object::SFrameParser<ESelectedEndian>;

        auto view = Y_LLVM_RAISE(sframeSection.getContents());
        auto parser = Y_LLVM_RAISE(TParser::create(llvm::arrayRefFromStringRef(view), sframeSection.getAddress()));
        auto fdeRows = Y_LLVM_RAISE(parser.fdes());

        for (auto it = fdeRows.begin(); it != fdeRows.end(); ++it) {
            const auto& fde = *it;
            if (fde.Info.getFDEType() != llvm::sframe::FDEType::PCInc) {
                continue;
            }

            uint64_t fdeStartAddress = parser.getAbsoluteStartAddress(it);
            llvm::Error err = llvm::Error::success();

            // Collect pcs for ranges
            llvm::SmallVector<uint64_t, 4> pcs;
            for (const auto& fre : parser.fres(fde, err)) {
                pcs.push_back(fre.StartAddress);
            }
            pcs.push_back(fde.Size);
            if (err) {
                throw TLLVMException{} << toString(std::move(err));
            }

            // Run handler
            size_t i = 0;
            for (const auto& fre : parser.fres(fde, err)) {
                TFdeFre data;

                data.fde.pc = fdeStartAddress;
                data.fde.size = fde.Size;

                data.fre.info = fre.Info.Info.value();
                data.fre.pc = fdeStartAddress + fre.StartAddress;
                data.fre.range = pcs[i + 1] - pcs[i];
                data.fre.offsets = llvm::SmallVector<int32_t>(fre.Offsets.begin(), fre.Offsets.end());

                handle(data);
                i++;
            }
            if (err) {
                throw TLLVMException{} << toString(std::move(err));
            }
        }
    }

    llvm::endianness getEndian(llvm::object::SectionRef sframeSection) {
        using THeader = llvm::sframe::Header<llvm::endianness::little>;
        Y_ENSURE(sframeSection.getSize() >= sizeof(THeader), "SFrame section smaller than SFrame Header.");

        auto view = Y_LLVM_RAISE(sframeSection.getContents());
        const char* section_start = view.data();

        THeader hdr;
        std::memcpy(&hdr, section_start, sizeof(hdr));

        bool isMagicOk = hdr.Preamble.Magic.value() == llvm::sframe::Magic;
        return isMagicOk ? llvm::endianness::little : llvm::endianness::big;
    }

    void handleSframeSection(llvm::object::SectionRef sframeSection, TFreHandler handle) {
        if (getEndian(sframeSection) == llvm::endianness::little) {
            iterateFdeImpl<llvm::endianness::little>(sframeSection, handle);
        } else {
            iterateFdeImpl<llvm::endianness::big>(sframeSection, handle);
        }
    }

} // anonymous namespace

void IterateOverSframeFre(llvm::object::ObjectFile* objectFile, TFreHandler handle) {
    constexpr auto SFRAME_SECTION_NAME = llvm::StringRef(".sframe");
    for (auto section : objectFile->sections()) {
        auto maybe_name = section.getName();
        if (!maybe_name) {
            continue;
        }
        if (*maybe_name == SFRAME_SECTION_NAME) {
            handleSframeSection(section, handle);
            break;
        }
    }
}
