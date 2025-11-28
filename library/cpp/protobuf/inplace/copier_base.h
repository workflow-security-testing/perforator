#pragma once

#include "common.h"
#include "parser.h"

#include <google/protobuf/io/coded_stream.h>
#include <contrib/libs/protobuf/src/google/protobuf/stubs/common.h>

#include <util/generic/vector.h>

namespace NInPlaceProto {

// general parser contract on TryField processing return bool value:
// true = need restart field search or exit due to corruption
// false = run next field processor

template <typename TSavedState, typename TContext>
class TCompositeContext : public TSavedState, public TContext {
public:
    template <typename... TArgs>
    TCompositeContext(TSavedState&& state, TArgs&&... args)
        : TSavedState(std::move(state))
        , TContext(std::forward<TArgs>(args)...)
    {
    }
};

template <bool copyByDefault>
class TCopySizeContext {
private:
    TVector<ui32>& SizesCache;
    ui32 Total = 0;
    const ui8* BeforeTag = nullptr;

public:
    TCopySizeContext(TVector<ui32>& sizesCache)
        : SizesCache(sizesCache)
    {
    }

    void ReadTag(THeavyParser& parser) {
        BeforeTag = parser.GetCurrentPos();
        parser.NextFieldNumber();
    }
    void ConsumeDefaultField(THeavyParser& parser) {
        parser.SkipMulti();
        if (copyByDefault) {
            const ui8* afterTag = parser.GetCurrentPos();
            Total += (afterTag - BeforeTag);
        }
        ReadTag(parser);
    }
    void ConsumeInvertedField(THeavyParser& parser) {
        parser.SkipMulti();
        if (!copyByDefault) {
            const ui8* afterTag = parser.GetCurrentPos();
            Total += (afterTag - BeforeTag);
        }
        ReadTag(parser);
    }

    ui32 GetTotal() const {
        return Total;
    }

    struct TMessageParseState {
        size_t TagSize;
        size_t Index;
        const TStringBuf SubRegion;
    };

    template <bool subCopyByDefault>
    using TMessageContext = TCompositeContext<TMessageParseState, TCopySizeContext<subCopyByDefault>>;

    template <bool subCopyByDefault>
    TMessageContext<subCopyByDefault> CreateSubContext(THeavyParser& parser) {
        const ui8* afterTag = parser.GetCurrentPos();
        ui32 tagSize = afterTag - BeforeTag;
        ui32 length = parser.ReadVarint32();
        auto region = parser.GetRegion(length);
        size_t index = SizesCache.size();
        SizesCache.push_back(0); // reserve space

        return {TMessageParseState{tagSize, index, TStringBuf(region.data(), region.size())}, SizesCache};
    }

    template <bool subCopyByDefault>
    void FinishSubContext(const TMessageContext<subCopyByDefault>& subContext, THeavyParser& parser) {
        ui32 payloadSize = subContext.GetTotal();
        SizesCache[subContext.Index] = payloadSize;
        Total += subContext.TagSize + TCodedOutputStream::VarintSize32(payloadSize) + payloadSize;
        ReadTag(parser);
    }
};

template <bool copyByDefault>
class TCopyContext {
private:
    ui8*& Data;
    const ui32*& Sizes;
    const ui8* BeforeTag = nullptr;

public:
    TCopyContext(ui8*& data, const ui32*& sizesPtr)
        : Data(data)
        , Sizes(sizesPtr)
    {
    }

    void ReadTag(THeavyParser& parser) {
        BeforeTag = parser.GetCurrentPos();
        parser.NextFieldNumber();
    }

    void ConsumeDefaultField(THeavyParser& parser) {
        parser.SkipMulti();
        if (copyByDefault) {
            const ui8* afterTag = parser.GetCurrentPos();
            size_t copied = afterTag - BeforeTag;
            memcpy(Data, BeforeTag, copied);
            Data += copied;
        }
        ReadTag(parser);
    }
    void ConsumeInvertedField(THeavyParser& parser) {
        parser.SkipMulti();
        if (!copyByDefault) {
            const ui8* afterTag = parser.GetCurrentPos();
            size_t copied = afterTag - BeforeTag;
            memcpy(Data, BeforeTag, copied);
            Data += copied;
        }
        ReadTag(parser);
    }

    struct TMessageParseState {
        const TStringBuf SubRegion;
    };

    template <bool subCopyByDefault>
    using TMessageContext = TCompositeContext<TMessageParseState, TCopyContext<subCopyByDefault>>;

    template <bool subCopyByDefault>
    TMessageContext<subCopyByDefault> CreateSubContext(THeavyParser& parser) {
        const ui8* afterTag = parser.GetCurrentPos();
        ui32 tagSize = afterTag - BeforeTag;
        ui32 length = parser.ReadVarint32();
        auto region = parser.GetRegion(length);

        // Copy tag
        memcpy(Data, BeforeTag, tagSize);
        Data += tagSize;

        // Serialize size
        ui32 payloadSize = *Sizes++;
        Data = TCodedOutputStream::WriteVarint32ToArray(payloadSize, Data);

        return {TMessageParseState{TStringBuf(region.data(), region.size())}, Data, Sizes};
    }

    template <bool subCopyByDefault>
    void FinishSubContext(const TMessageContext<subCopyByDefault>& /*subContext*/, THeavyParser& parser) {
        ReadTag(parser);
    }
};

template <typename TDerived, typename TField>
class TCopierGlueBase {
public:
    template <typename TCopySizeContext>
    static inline bool TryCopy(TCopySizeContext& context, THeavyParser& parser) {
        // Must work only for first. Skip fields before current
        if (parser.GetFieldNumber() < TField::TagId) {
            context.ConsumeDefaultField(parser);
            return true; // Restart going through list of fields
        }
        while (Y_LIKELY(parser.GetFieldNumber() == TField::TagId)) {
            if (TField::TryCopy(context, parser)) {
                return true;
            }
        }
        while (parser.GetFieldNumber() > TField::TagId) {
            if (TDerived::FieldNumberTooLarge(parser.GetFieldNumber())) {
                // continue processing with next field
                return false;
            }
            context.ConsumeDefaultField(parser);
        }
        return true;
    }
};

template <typename TField1, typename TField2>
class TCopierGlue : public TCopierGlueBase<TCopierGlue<TField1, TField2>, TField1> {
public:
    static_assert(TField1::TagId < TField2::TagId, "Fields tag ids must increase");

    static inline bool FieldNumberTooLarge(ui32 fieldNumber) noexcept {
        return fieldNumber >= TField2::TagId;
    }
};

template <typename TField>
class TCopierGlue<TField, void> : public TCopierGlueBase<TCopierGlue<TField, void>, TField> {
public:
    static inline bool FieldNumberTooLarge(ui32 /*fieldNumber*/) noexcept {
        return false;
    }
};

template <typename... TFields>
class TCombinedCopier {
private:
    template <typename TCopyContext>
    static inline bool SkipField(TCopyContext& copyContext, THeavyParser& parser) {
        copyContext.ConsumeDefaultField(parser);
        return true;
    }

public:
    template <typename TCopyContext>
    static inline bool Copy(TCopyContext& copyContext, THeavyParser& parser) {
        while (parser.GetFieldNumber()) {
            (TFields::TryCopy(copyContext, parser) || ... || SkipField(copyContext, parser));
            if (parser.IsCorrupted()) {
                return false;
            }
        }
        return !parser.IsCorrupted();
    }
};

template <typename TBase, bool copyByDefault>
class TCopier {
private:
    template <typename TCopyContext>
    static inline bool ApplyStage(TCopyContext& copyContext, TStringBuf source) {
        THeavyParser parser(source.data(), source.size());
        copyContext.ReadTag(parser);
        return TBase::TFieldList::Copy(copyContext, parser);
    }

protected:
    class TInvertedConsumer {
    public:
        template <typename TCopyContext>
        static inline bool TryCopy(TCopyContext& copyContext, THeavyParser& parser) {
            copyContext.ConsumeInvertedField(parser);
            return parser.IsCorrupted();
        }
    };

    template <typename... TFields>
    using TFieldListImpl = typename TCombinedList<TCombinedCopier, TCopierGlue, TTypeList<>, TFields...>::TResult;

public:
    // For external usage only. For sub messages. Creates save point & sub parser/context
    template <typename TCopyContext>
    static inline bool TryCopy(TCopyContext& copyContext, THeavyParser& parser) {
        auto subContext = copyContext.template CreateSubContext<copyByDefault>(parser);
        if (Y_UNLIKELY(parser.IsCorrupted())) {
            return true;
        }
        bool success = ApplyStage(subContext, subContext.SubRegion);
        if (Y_LIKELY(success)) {
            copyContext.FinishSubContext(subContext, parser);
        }

        // Due to return value policy we need to return true *on errors*
        // So, inverted value
        return !success;
    }

    static inline bool Apply(TStringBuf source, TString& result) {
        TVector<ui32> sizeCache;
        TCopySizeContext<copyByDefault> copySize(sizeCache);
        if (ApplyStage(copySize, source)) {
            TString ret;
            ret.ReserveAndResize(copySize.GetTotal()); // Ugly hack #1

            const ui32* sizesPtr = sizeCache.data();
            ui8* data = (ui8*)&ret[0]; // Ugly hack #2
            TCopyContext<copyByDefault> copyContext(data, sizesPtr);
            bool success = ApplyStage(copyContext, source);
            Y_ASSERT(success); // All errors must be catched on size calculation phase

            result = std::move(ret);
            return true;
        }
        return false;
    }
};

} // namespace NInPlaceProto
