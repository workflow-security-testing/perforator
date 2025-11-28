#pragma once

#include "common.h"
#include "macro_for_header.h"
#include "parser.h"
#include "region_data_provider.h"

#include <util/generic/adaptor.h>
#include <util/generic/cast.h>
#include <util/generic/function.h>
#include <util/generic/fwd.h>
#include <util/generic/vector.h>
#include <util/system/types.h>
#include <util/system/yassert.h>

namespace NInPlaceProto {

// Traits
template <typename T>
class TProtoFieldTraits {
public:
    using TSubMessage = T;
    enum {
        Repeated = false
    };
};

// complex repeated
template <typename T>
class TProtoFieldTraits<::google::protobuf::RepeatedPtrField<T>> {
public:
    using TSubMessage = T;
    enum {
        Repeated = true
    };
};

// basic repeated
template <typename T>
class TProtoFieldTraits<::google::protobuf::RepeatedField<T>> {
public:
    using TSubMessage = T;
    enum {
        Repeated = true
    };
};


template <typename T>
class TClassFieldTraits {
protected:
    static inline T& GetFieldImpl(T& value) {
        return value;
    }

public:
    using TFieldType = T;

    enum {
        Container = false,
        DirectMemoryOrder = true,
        UseTmpField = false
    };
};

template <typename T>
class TClassFieldTraits<TVector<T>> {
protected:
    static inline size_t GetSizeImpl(const TVector<T>& values) noexcept {
        return values.size();
    }

    static inline void AddValueImpl(TVector<T>& values, T&& value) {
        values.emplace_back(std::move(value));
    }

    template <typename TFunc>
    static inline void ReverseForEachImpl(const TVector<T>& values, TFunc&& func) {
        for (const T& value : ::Reversed(values)) {
            func(value);
        }
    }
    template <typename TFunc>
    static inline void ForEachImpl(const TVector<T>& values, TFunc&& func) {
        for (const T& value : values) {
            func(value);
        }
    }

    // Only for little-endian systems. Important optimization. Gives huge bonus
    static inline ui8* CopyRawImpl(const TVector<T>& values, ui32 payloadSize, ui8* current) {
        return TCodedOutputStream::WriteRawToArray(values.data(), payloadSize, current);
    }
    static inline void FillRawImpl(TVector<T>& values, const T* data, ui32 size) {
        values.insert(values.end(), data, data + size);
    }

    template <typename TFunc>
    static inline void OnLastElement(const TVector<T>& values, TFunc&& func) {
        if (values) {
            func(values.back());
        }
    }

public:
    using TFieldType = T;

    enum {
        Container = true,
        DirectMemoryOrder = true,
        UseTmpField = true
    };
};

template <typename T>
class TClassFieldTraits<TMaybe<T>> {
protected:
    static inline size_t GetSizeImpl(const TMaybe<T>& values) noexcept {
        return values.Defined();
    }

    static inline void AddValueImpl(TMaybe<T>& values, T&& value) {
        values = std::move(value);
    }
    template <typename TFunc>
    static inline void ForEachImpl(const TVector<T>& values, TFunc&& func) {
        if (values.Defined()) {
            func(values.GetRef());
        }
    }
    template <typename TFunc>
    static inline void ReverseForEachImpl(const TMaybe<T>& values, TFunc&& func) {
        ForEach(values, std::forward<TFunc>(func));
    }

    template <typename TFunc>
    static inline void OnLastElement(const TMaybe<T>& values, TFunc&& func) {
        ForEach(values, std::forward<TFunc>(func));
    }

    static inline T& GetFieldImpl(TMaybe<T>& values) {
        values.ConstructInPlace();
        return values.GetRef();
    }

public:
    using TFieldType = T;

    enum {
        Container = true,
        DirectMemoryOrder = false,
        UseTmpField = false
    };
};


template <typename TObject, typename TField, typename TAccessor>
class TAssignAction : public TClassFieldTraits<TField> {
private:
    using TBase = TClassFieldTraits<TField>;

    static inline bool OnValueImpl(TField& field, typename TBase::TFieldType&& value) {
        if constexpr (TBase::Container) {
            TBase::AddValueImpl(field, std::move(value));
        } else {
            field = std::move(value);
        }
        return true;
    }

public:
    using TFieldType = typename TBase::TFieldType;

    static inline bool OnValue(TObject& object, TFieldType&& value) {
        return OnValueImpl(TAccessor::GetMutableField(object), std::move(value));
    }

    static inline TFieldType& GetField(TObject& object) {
        return TBase::GetFieldImpl(TAccessor::GetMutableField(object));
    }

    static inline bool OnBatchValue(TObject& object, const TFieldType* data, ui32 size) {
        auto& field = TAccessor::GetMutableField(object);
        if constexpr (TBase::DirectMemoryOrder) {
            TBase::FillRawImpl(field, data, size);
        } else {
            auto& field = TAccessor::GetMutableField(object);
            const TFieldType*const end = data + size;
            for (const TFieldType* cur = data; cur < end; ++cur) {
                OnValueImpl(field, *cur);
            }
        }
        return true;
    }

    static inline size_t GetSize(const TObject& object) noexcept {
        return TBase::GetSizeImpl(TAccessor::GetConstField(object));
    }

    template <typename TFunc>
    static inline void ForEach(const TObject& object, TFunc&& func) {
        TBase::ForEachImpl(TAccessor::GetConstField(object), std::forward<TFunc>(func));
    }

    template <typename TFunc>
    static inline void ReverseForEach(const TObject& object, TFunc&& func) {
        TBase::ReverseForEachImpl(TAccessor::GetConstField(object), std::forward<TFunc>(func));
    }

    static inline ui8* CopyRaw(const TObject& object, ui32 payloadSize, ui8* current) {
        return TBase::CopyRawImpl(TAccessor::GetConstField(object), payloadSize, current);
    }

    template <typename TFunc>
    static inline void OnLastElement(const TObject& object, TFunc&& func) {
        const auto& field = TAccessor::GetConstField(object);
        if constexpr (TBase::Container) {
            TBase::OnLastElement(field, std::forward<TFunc>(func));
        } else {
            func(field);
        }
    }
};

template <typename TObject, typename TField, typename TDerived>
class TCustomAction {
public:
    using TFieldType = TField;

    enum {
        UseTmpField = true,
        DirectMemoryOrder = false // TODO: make castomization
    };

    static inline bool OnBatchValue(TObject& object, const TFieldType* data, ui32 size) {
        const TFieldType*const end = data + size;
        for (const TFieldType* cur = data; cur < end; ++cur) {
            if (!TDerived::OnValue(object, *cur)) {
                return false;
            }
        }
        return true;
    }

    template <typename TFunc>
    static inline void ForEach(const TObject& object, TFunc&& func) {
        size_t index = TDerived::GetSize(object);
        for (size_t i = 0; i < index; ++i) {
            func(TDerived::GetValue(object, i));
        }
    }

    template <typename TFunc>
    static inline void ReverseForEach(const TObject& object, TFunc&& func) {
        size_t index = TDerived::GetSize(object);
        for (size_t i = index; i > 0; --i) {
            func(TDerived::GetValue(object, i - 1));
        }
    }

    template <typename TFunc>
    static inline void OnLastElement(const TObject& object, TFunc&& func) {
        func(TDerived::GetValue(object));
    }
};


template <ui32 tagId, TWireType wireType>
class TTagSerializer {
public:
    constexpr static inline size_t GetSize() noexcept {
        const ui32 tag = TWireFormatLite::MakeTag(tagId, wireType);
        return TCodedOutputStream::VarintSize32(tag);
    }
    static inline ui8* Serialize(ui8* current) noexcept {
        const ui32 tag = TWireFormatLite::MakeTag(tagId, wireType);
        return TCodedOutputStream::WriteVarint32ToArray(tag, current);
    }
};

// Combining
template <typename TObject, typename TObject2MessageTraits, bool useProto2Syntax, bool needParser, bool needSerializer>
class TBaseParserOrSerializer;

template <typename TObject, typename... TSerializers>
class TCombinedSerializer;

template <typename TObject, typename TSerializer, typename... TSerializers>
class TCombinedSerializer<TObject, TSerializer, TSerializers...> {
private:
    using TNextSerializer = TCombinedSerializer<TObject, TSerializers...>;

public:
    static inline size_t GetSerializedSize(const TObject& object, TSizeStack& stack) {
        // Reverse order size caching!
        size_t ret = TNextSerializer::GetSerializedSize(object, stack);
        return ret + TSerializer::GetSerializedSize(object, stack);
    }

    static inline ui8* SerializeWithCachedSizes(const TObject& object, TSizeStack& stack, ui8* current) {
        // Serialize in direct order!
        current = TSerializer::SerializeWithCachedSizes(object, stack, current);
        return TNextSerializer::SerializeWithCachedSizes(object, stack, current);
    }
};

template <typename TObject>
class TCombinedSerializer<TObject> {
public:
    static inline size_t GetSerializedSize(const TObject& /*object*/, TSizeStack& /*stack*/) noexcept {
        return 0;
    }
    static inline ui8* SerializeWithCachedSizes(const TObject& /*object*/, TSizeStack& /*stack*/, ui8* current) noexcept {
        return current;
    }
};

template <typename TObject, typename... TSerializers>
class TCombinedParser {
private:
    // Must never be called if TParserGlue is used
    static inline bool SkipField(THeavyParser& parser) {
        parser.SkipMulti();
        parser.NextFieldNumber();
        return true;
    }

public:
    static inline bool ParseFromStringBuf(TObject& object, TStringBuf data) {
        THeavyParser parser(data.data(), data.size());
        parser.NextFieldNumber();
        while (parser.GetFieldNumber()) {
            (TSerializers::TryParse(object, parser) || ... || SkipField(parser));
            if (Y_UNLIKELY(parser.IsCorrupted())) {
                return false;
            }
        }
        return !parser.IsCorrupted();
    }
};

template <typename TObject, typename... TSerializers>
class TCombinedParserAndSerializer :
    public TCombinedParser<TObject, TSerializers...>,
    public TCombinedSerializer<TObject, TSerializers...>
{
};

class TWireTypeTags {
public:
    template <TWireType wireType>
    class TWireTypeTag {
    public:
        static constexpr TWireType WireType = wireType;
    };

    class Tdouble : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED64> {
    };
    class Tfloat : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED32> {
    };

    class Tint32 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tsint32 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tsfixed32 : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED32> {
    };

    class Tint64 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tsint64 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tsfixed64 : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED64> {
    };

    class Tuint32 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tfixed32 : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED32> {
    };

    class Tuint64 : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };
    class Tfixed64 : public TWireTypeTag<TWireFormatLite::WIRETYPE_FIXED64> {
    };

    class Tbool : public TWireTypeTag<TWireFormatLite::WIRETYPE_VARINT> {
    };

    class Tstring : public TWireTypeTag<TWireFormatLite::WIRETYPE_LENGTH_DELIMITED> {
    };
    class Tbytes : public TWireTypeTag<TWireFormatLite::WIRETYPE_LENGTH_DELIMITED> {
    };
};

template <typename TCppFieldType, typename TWireCppType>
class TValueAdapterBitCast {
public:
    static inline TWireCppType ReadValue(TCppFieldType value) noexcept {
        return BitCast<TWireCppType>(value);
    }
    static inline bool ParseValue(TCppFieldType& field, TWireCppType wireValue) noexcept {
        field = BitCast<TCppFieldType>(wireValue);
        return true;
    }
};

template <typename TCppFieldType, typename TWriteTypeTagOrTraits, bool useProto2Syntax>
class TValueAdapter;

template <bool useProto2Syntax>
class TValueAdapter<double, TWireTypeTags::Tdouble, useProto2Syntax> : public TValueAdapterBitCast<double, ui64> {
};

template <bool useProto2Syntax>
class TValueAdapter<float, TWireTypeTags::Tfloat, useProto2Syntax> : public TValueAdapterBitCast<float, ui32> {
};

template <typename TCppFieldType, typename TWireCppType>
class TValueAdapterDirectCast {
public:
    static inline TWireCppType ReadValue(TCppFieldType value) noexcept {
        return (TWireCppType)value;
    }
    static inline bool ParseValue(TCppFieldType& field, TWireCppType wireValue) noexcept {
        field = (TCppFieldType)wireValue;
        return true;
    }
};

template <typename TCppFieldType>
using TValueAdapterNoCast = TValueAdapterDirectCast<TCppFieldType, TCppFieldType>;

template <bool useProto2Syntax>
class TValueAdapter<i32, TWireTypeTags::Tint32, useProto2Syntax> {
public:
    static inline ui64 ReadValue(i32 value) noexcept {
        return (ui64)(i64)value;
    }
    static inline bool ParseValue(i32& field, ui32 wireValue) noexcept {
        field = (i32)wireValue;
        return true;
    }
};

template <bool useProto2Syntax>
class TValueAdapter<i32, TWireTypeTags::Tsint32, useProto2Syntax> {
public:
    static inline ui32 ReadValue(i32 value) noexcept {
        return TWireFormatLite::ZigZagEncode32(value);
    }
    static inline bool ParseValue(i32& field, ui32 wireValue) noexcept {
        field = TWireFormatLite::ZigZagDecode32(wireValue);
        return true;
    }
};

template <bool useProto2Syntax>
class TValueAdapter<i32, TWireTypeTags::Tsfixed32, useProto2Syntax> : public TValueAdapterDirectCast<i32, ui32> {
};

template <bool useProto2Syntax>
class TValueAdapter<i64, TWireTypeTags::Tint64, useProto2Syntax> : public TValueAdapterDirectCast<i64, ui64> {
};

template <bool useProto2Syntax>
class TValueAdapter<i64, TWireTypeTags::Tsint64, useProto2Syntax> {
public:
    static inline ui64 ReadValue(i64 value) noexcept {
        return TWireFormatLite::ZigZagEncode64(value);
    }
    static inline bool ParseValue(i64& field, ui64 wireValue) noexcept {
        field = TWireFormatLite::ZigZagDecode64(wireValue);
        return true;
    }
};

template <bool useProto2Syntax>
class TValueAdapter<i64, TWireTypeTags::Tsfixed64, useProto2Syntax> : public TValueAdapterDirectCast<i64, ui64> {
};

template <bool useProto2Syntax>
class TValueAdapter<ui32, TWireTypeTags::Tuint32, useProto2Syntax> : public TValueAdapterNoCast<ui32> {
};

template <bool useProto2Syntax>
class TValueAdapter<ui32, TWireTypeTags::Tfixed32, useProto2Syntax> : public TValueAdapterNoCast<ui32> {
};

template <bool useProto2Syntax>
class TValueAdapter<ui64, TWireTypeTags::Tuint64, useProto2Syntax> : public TValueAdapterNoCast<ui64> {
};

template <bool useProto2Syntax>
class TValueAdapter<ui64, TWireTypeTags::Tfixed64, useProto2Syntax> : public TValueAdapterNoCast<ui64> {
};

template <bool useProto2Syntax>
class TValueAdapter<bool, TWireTypeTags::Tbool, useProto2Syntax> : public TValueAdapterNoCast<bool> {
};

template <bool useProto2Syntax>
class TValueAdapter<TString, TWireTypeTags::Tstring, useProto2Syntax> {
public:
    static inline TStringBuf ReadValue(const TString& value) noexcept {
        return value;
    }
    static inline bool ParseValue(TString& field, TStringBuf wireValue) noexcept {
        field = wireValue;
        return useProto2Syntax || ::google::protobuf::internal::IsStructurallyValidUTF8(field.data(), field.size());;
    }
};
template <bool useProto2Syntax>
class TValueAdapter<TStringBuf, TWireTypeTags::Tstring, useProto2Syntax> {
public:
    static inline TStringBuf ReadValue(const TStringBuf& value) noexcept {
        return value;
    }
    static inline bool ParseValue(TStringBuf& field, TStringBuf wireValue) noexcept {
        field = wireValue;
        return useProto2Syntax || ::google::protobuf::internal::IsStructurallyValidUTF8(field.data(), field.size());;
    }
};

template <bool useProto2Syntax>
class TValueAdapter<TString, TWireTypeTags::Tbytes, useProto2Syntax> {
public:
    static inline TStringBuf ReadValue(const TString& value) noexcept {
        return value;
    }
    static inline bool ParseValue(TString& field, TStringBuf wireValue) noexcept {
        field = wireValue;
        return true;
    }
};

template <bool useProto2Syntax>
class TValueAdapter<TStringBuf, TWireTypeTags::Tbytes, useProto2Syntax> {
public:
    static inline TStringBuf ReadValue(const TStringBuf& value) noexcept {
        return value;
    }
    static inline bool ParseValue(TStringBuf& field, TStringBuf wireValue) noexcept {
        field = wireValue;
        return true;
    }
};

template <typename TCppObject, bool useProto2Syntax>
class TFieldCollection {
protected:
    template <
        typename TAction,
        ui32 tagId,
        typename TWireTypeTag, // Serializer for messages
        typename TProtoMessageGetResult,
        bool packed
    >
    class TFieldImpl {
    public:
        using TObject = TCppObject;
        using TFieldType = typename TAction::TFieldType;
        using TMessageFieldTraits = TProtoFieldTraits<std::decay_t<TProtoMessageGetResult>>;
        using TMainTagSerializer = TTagSerializer<tagId, TWireTypeTag::WireType>;
        using TPackedTagSerializer = TTagSerializer<tagId, TWireFormatLite::WIRETYPE_LENGTH_DELIMITED>;
        enum {
            TagId = tagId,
            UseProto2Syntax = useProto2Syntax,
            Numeric = TWireTypeTag::WireType != TWireFormatLite::WIRETYPE_LENGTH_DELIMITED,
            RealPacked = Numeric && (!useProto2Syntax && TMessageFieldTraits::Repeated || useProto2Syntax && packed),
            CanBePacked = Numeric && TMessageFieldTraits::Repeated,
            Fixed64 = TWireTypeTag::WireType == TWireFormatLite::WIRETYPE_FIXED64,
            Fixed32 = TWireTypeTag::WireType == TWireFormatLite::WIRETYPE_FIXED32,
            Varint = TWireTypeTag::WireType == TWireFormatLite::WIRETYPE_VARINT,
            Fixed = Fixed64 || Fixed32,
            Boolean = std::is_same_v<TWireTypeTags::Tbool, TWireTypeTag>,
            FixedSize = Fixed64 ? 8 : (Fixed32 ? 4  : (Boolean ? 1 : 0)),
            String = TTypeList<TWireTypeTags::Tstring, TWireTypeTags::Tbytes>::THave<TWireTypeTag>::value,
            Message = !Numeric && !String,
            Repeated = TMessageFieldTraits::Repeated,
#if defined(_little_endian_)
            CanDirectCopy = Fixed,
#else
            CanDirectCopy = false,
#endif
        };
        using TValueAdapter = std::conditional_t<Message, TWireTypeTag, TValueAdapter<TFieldType, TWireTypeTag, useProto2Syntax>>;

        // Many asserts to get porblems fastera with descriptive messages
        static_assert(Repeated || !packed, "Only repeated fields are packed");
        static_assert(UseProto2Syntax || !packed, "Only proto2 syntax have explicit packed");
        static_assert(Numeric || !packed, "Only basic numeric types are packed");

    private:
        static inline size_t GetVarintSizeImpl(const bool& /*value*/) {
            return 1;
        }
        static inline size_t GetVarintSizeImpl(const ui32& value) {
            return TCodedOutputStream::VarintSize32(value);
        }
        static inline size_t GetVarintSizeImpl(const ui64& value) {
            return TCodedOutputStream::VarintSize64(value);
        }
        static inline size_t GetSerializedSizeImpl(const TFieldType& value, TSizeStack& stack) {
            if constexpr (Fixed || Boolean) {
                return FixedSize;
            } else if constexpr (Varint) {
                return GetVarintSizeImpl(TValueAdapter::ReadValue(value));
            } else if constexpr (String) {
                ui32 payloadSize = TValueAdapter::ReadValue(value).size();
                return TCodedOutputStream::VarintSize32(payloadSize) + payloadSize;
            } else {
                static_assert(Message, "Missed case!");
                ui32 payloadSize = TWireTypeTag::GetSerializedSize(value, stack);
                stack.PushSize(payloadSize);
                return TCodedOutputStream::VarintSize32(payloadSize) + payloadSize;
            }
        }
        static inline size_t GetSerializedSizeWithTagImpl(const TFieldType& value, TSizeStack& stack) {
            return TMainTagSerializer::GetSize() + GetSerializedSizeImpl(value, stack);
        }

        static inline ui8* SerializeVarintImpl(const bool& value, ui8* current) {
            *current++ = value;
            return current;
        }
        static inline ui8* SerializeVarintImpl(const ui32& value, ui8* current) {
            return TCodedOutputStream::WriteVarint32ToArray(value, current);
        }
        static inline ui8* SerializeVarintImpl(const ui64& value, ui8* current) {
            return TCodedOutputStream::WriteVarint64ToArray(value, current);
        }
        static inline ui8* SerializeImpl(const TFieldType& value, TSizeStack& stack, ui8* current) {
            if constexpr (Varint) {
                return SerializeVarintImpl(TValueAdapter::ReadValue(value), current);
            } else if constexpr (Fixed64) {
                return TCodedOutputStream::WriteLittleEndian64ToArray(TValueAdapter::ReadValue(value), current);
            } else if constexpr (Fixed32) {
                return TCodedOutputStream::WriteLittleEndian32ToArray(TValueAdapter::ReadValue(value), current);
            } else if constexpr (String) {
                const auto& buf = TValueAdapter::ReadValue(value);
                ui32 payloadSize = buf.size();
                current = TCodedOutputStream::WriteVarint32ToArray(payloadSize, current);
                return TCodedOutputStream::WriteRawToArray(buf.data(), payloadSize, current);
            } else {
                static_assert(Message, "Missed case!");
                size_t payloadSize = stack.PopSize();
                current = TCodedOutputStream::WriteVarint32ToArray(payloadSize, current);
                return TWireTypeTag::SerializeWithCachedSizes(value, stack, current);
            }
        }
        static inline ui8* SerializeWithTagImpl(const TFieldType& value, TSizeStack& stack, ui8* current) {
            current = TMainTagSerializer::Serialize(current);
            return SerializeImpl(value, stack, current);
        }

        static inline bool ParseImpl2(TFieldType& value, TLightParser& parser) {
            if constexpr (Varint) {
                using TParseArg = TFunctionArg<decltype(TValueAdapter::ParseValue), 1>;
                if constexpr (std::is_same_v<TParseArg, ui64>) {
                    return TValueAdapter::ParseValue(value, parser.ReadVarint64());
                } else if constexpr (std::is_same_v<TParseArg, ui32>) {
                    return TValueAdapter::ParseValue(value, parser.ReadVarint32());
                } else {
                    static_assert(std::is_same_v<TParseArg, bool>, "Bad varint TValueAdapter!");
                    return TValueAdapter::ParseValue(value, parser.ReadVarintBool());
                }
            } else if constexpr (Fixed64) {
                return TValueAdapter::ParseValue(value, parser.ReadLittleEndian64());
            } else if constexpr (Fixed32) {
                return TValueAdapter::ParseValue(value, parser.ReadLittleEndian32());
            } else {
                ui32 length = parser.ReadVarint32();
                auto region = parser.GetRegion(length);
                TStringBuf buf(region.data(), region.size());
                if (parser.IsCorrupted()) {
                    return false;
                }
                if constexpr (String) {
                    return TValueAdapter::ParseValue(value, buf);
                } else {
                    static_assert(Message, "Missed case!");
                    return TWireTypeTag::ParseFromStringBuf(value, buf);
                }
            }
        }

        static inline bool ParseImpl(TObject& object, TLightParser& parser) {
            if constexpr (TAction::UseTmpField) {
                TFieldType value;
                bool ret = ParseImpl2(value, parser);
                // Allows TValueAdapter & TAction to just return false in struggling cases
                if (ret && TAction::OnValue(object, std::move(value))) {
                    return true;
                }
            } else {
                if (ParseImpl2(TAction::GetField(object), parser)) {
                    return true;
                }
            }
            parser.SetCorrupted();
            return false;
        }

    public:
        static inline size_t GetSerializedSize(const TObject& object, TSizeStack& stack) {
            if constexpr (RealPacked) {
                // Packed encoding for fixed
                size_t size = TAction::GetSize(object);
                if (!size) {
                    return 0;
                }
                ui32 payloadSize = 0;
                if constexpr (Fixed || Boolean) {
                    payloadSize = FixedSize * size;
                } else {
                    TAction::ReverseForEach(object, [&] (const TFieldType& value) {
                        payloadSize += GetSerializedSizeImpl(value, stack);
                    });
                    stack.PushSize(payloadSize);
                }
                return TPackedTagSerializer::GetSize() + TCodedOutputStream::VarintSize32(payloadSize) + payloadSize;
            } else if constexpr (Repeated) {
                // Repeated enconding for fixed (not packed)
                if constexpr (Fixed || Boolean) {
                    return (TMainTagSerializer::GetSize() + FixedSize) * TAction::GetSize(object);
                } else {
                    ui32 ret = 0;
                    TAction::ReverseForEach(object, [&] (const TFieldType& value) {
                        ret += GetSerializedSizeWithTagImpl(value, stack);
                    });
                    return ret;
                }
            } else {
                // Single element. No packed
                ui32 ret = 0;
                TAction::OnLastElement(object, [&] (const TFieldType& value) {
                    if constexpr (!UseProto2Syntax && !Message) {
                        if (!value) {
                            return;
                        }
                    }
                    ret += GetSerializedSizeWithTagImpl(value, stack);
                });
                return ret;
            }
        }
        static inline ui8* SerializeWithCachedSizes(const TObject& object, TSizeStack& stack, ui8* current) {
            if constexpr (RealPacked) {
                // Packed encoding for fixed
                size_t size = TAction::GetSize(object);
                if (!size) {
                    return current;
                }
                current = TPackedTagSerializer::Serialize(current);
                ui32 payloadSize;
                if constexpr (Fixed || Boolean) {
                    payloadSize = FixedSize * size;
                } else {
                    payloadSize = stack.PopSize();
                }
                current = TCodedOutputStream::WriteVarint32ToArray(payloadSize, current);
                if constexpr (CanDirectCopy && TAction::DirectMemoryOrder) {
                    current = TAction::CopyRaw(object, payloadSize, current);
                } else {
                    TAction::ForEach(object, [&] (const TFieldType& value) {
                        current = SerializeImpl(value, stack, current);
                    });
                }
                return current;
            } else if constexpr (Repeated) {
                // Repeated enconding for fixed (not packed)
                TAction::ForEach(object, [&] (const TFieldType& value) {
                    current = SerializeWithTagImpl(value, stack, current);
                });
                return current;
            } else {
                // Single element. No packed
                TAction::OnLastElement(object, [&] (const TFieldType& value) {
                    if constexpr (!UseProto2Syntax && !Message) {
                        if (!value) {
                            return;
                        }
                    }
                    current = SerializeWithTagImpl(value, stack, current);
                });
                return current;
            }
        }

        // general parser contract on return value:
        // true = need restart field search or exit due to corruption
        // false = run next field processor
        static inline bool TryParse(TObject& object, THeavyParser& parser) {
            Y_ASSERT(parser.GetFieldNumber() == tagId); // Must be true due to TParserGlue invariants
            auto wireType = parser.GetWireType();
            if constexpr (CanBePacked) {
                if (Y_LIKELY(wireType == TWireFormatLite::WIRETYPE_LENGTH_DELIMITED)) {
                    size_t length = parser.ReadVarint32();
                    if constexpr (Fixed) {
                        if (length % FixedSize != 0) {
                            parser.SetCorrupted();
                            return true;
                        }
                    }
                    auto region = parser.GetRegion(length);
                    if constexpr (CanDirectCopy && TAction::DirectMemoryOrder) {
                        if (!TAction::OnBatchValue(object, (const TFieldType*)region.data(), length / sizeof(TFieldType))) {
                            parser.SetCorrupted();
                            return true;
                        }
                    } else {
                        TLightParser fieldParser(region.data(), region.size());
                        while (fieldParser.NotEmpty()) {
                            if (!ParseImpl(object, fieldParser)) {
                                // corrupted
                                return true;
                            }
                        }
                    }
                    parser.NextFieldNumber();
                    return parser.GetFieldNumber() < tagId;
                }
            }
            if (Y_LIKELY(wireType == TWireTypeTag::WireType)) {
                if constexpr (Repeated) {
                    while (true) {
                        if (!ParseImpl(object, parser)) {
                            // corrupted
                            return true;
                        }
                        if constexpr (Repeated && tagId < 2048) {
                            bool match = parser.PeekTagId2048<tagId, TWireTypeTag::WireType>();
                            if (Y_LIKELY(match)) {
                                continue;
                            }
                        }
                        parser.NextFieldNumber();
                        if constexpr (Repeated && tagId >= 2048) {
                            if (Y_LIKELY(parser.GetFieldNumber() == tagId && parser.GetWireType() == TWireTypeTag::WireType)) {
                                continue;
                            }
                        }
                        break;
                    }
                } else {
                    if (!ParseImpl(object, parser)) {
                        // Corrupted
                        return true;
                    }
                    parser.NextFieldNumber();
                }
            } else {
                parser.SkipMulti();
                parser.NextFieldNumber();
            }
            return parser.GetFieldNumber() < tagId;
        }
    };
};

template <typename TObject, typename TObject2MessageTraits>
class TTraitsInfoPublisher {
protected:
    static inline size_t GetSerializedSize(const TObject& object, TSizeStack& stack) {
        return TObject2MessageTraits::TFieldList::GetSerializedSize(object, stack);
    }
    static inline ui8* SerializeWithCachedSizes(const TObject& object, TSizeStack& stack, ui8* current) {
        return TObject2MessageTraits::TFieldList::SerializeWithCachedSizes(object, stack, current);
    }
    static inline bool ParseFromStringBuf(TObject& object, TStringBuf data) {
        bool ret = TObject2MessageTraits::TFieldList::ParseFromStringBuf(object, data);
        if (ret) {
            TObject2MessageTraits::template TAfterInitAction<true>::Do(object);
        }
        return ret;
    }

public:
    static constexpr TWireType WireType = TWireFormatLite::WIRETYPE_LENGTH_DELIMITED;
};

// Serializer
template <typename TObject, typename TObject2MessageTraits, bool useProto2Syntax>
class TBaseParserOrSerializer<TObject, TObject2MessageTraits, useProto2Syntax, false, true> :
    public TWireTypeTags,
    public TFieldCollection<TObject, useProto2Syntax>,
    public TTraitsInfoPublisher<TObject, TObject2MessageTraits>
{
protected:
    template <typename... TFields>
    using TCombinedWithoutObject = TCombinedSerializer<TObject, TFields...>;

    template <typename... TFields>
    using TFieldListImpl = typename TCombinedList<TCombinedWithoutObject, TCheckTagOrder, TTypeList<>, TFields...>::TResult;

public:
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::GetSerializedSize;
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::SerializeWithCachedSizes;
};

// Parser
template <typename TDerived, typename TField>
class TParserGlueBase {
public:
    // general parser contract on return value:
    // true = need restart field search or exit due to corruption
    // false = run next field processor
    template <typename TObject>
    static inline bool TryParse(TObject& object, THeavyParser& parser) {
        // Must work only for first. Skip fields before current
        if (Y_UNLIKELY(parser.GetFieldNumber() < TField::TagId)) {
            parser.SkipMulti();
            parser.NextFieldNumber();
            return true; // Restart going through list of fields
        }
        if (Y_LIKELY(parser.GetFieldNumber() == TField::TagId)) {
            // Repeat cycles on wire type basis
            if (TField::TryParse(object, parser)) {
                return true;
            }
        }
        while (Y_LIKELY(parser.GetFieldNumber() > TField::TagId)) {
            if (Y_LIKELY(TDerived::FieldNumberTooLarge(parser.GetFieldNumber()))) {
                // continue processing with next field
                return false;
            }
            parser.SkipMulti();
            parser.NextFieldNumber();
        }
        return true;
    }
};

template <typename TField1, typename TField2>
class TParserGlue : public TParserGlueBase<TParserGlue<TField1, TField2>, TField1> {
public:
    static_assert(TField1::TagId < TField2::TagId, "Fields tag ids must increase");

    static inline bool FieldNumberTooLarge(ui32 fieldNumber) noexcept {
        return fieldNumber >= TField2::TagId;
    }
};

template <typename TField>
class TParserGlue<TField, void> : public TParserGlueBase<TParserGlue<TField, void>, TField> {
public:
    static inline bool FieldNumberTooLarge(ui32 /*fieldNumber*/) noexcept {
        return false;
    }
};

template <typename T1, typename T2>
class TParserAndSerializerGlue : public TParserGlue<T1, T2> {
public:
    template <typename TObject>
    static inline size_t GetSerializedSize(const TObject& object, TSizeStack& stack) {
        return T1::GetSerializedSize(object, stack);
    }

    template <typename TObject>
    static inline ui8* SerializeWithCachedSizes(const TObject& object, TSizeStack& stack, ui8* current) {
        return T1::SerializeWithCachedSizes(object, stack, current);
    }
};

template <typename TObject, typename TObject2MessageTraits, bool useProto2Syntax>
class TBaseParserOrSerializer<TObject, TObject2MessageTraits, useProto2Syntax, true, false> :
    protected TWireTypeTags,
    public TFieldCollection<TObject, useProto2Syntax>,
    public TTraitsInfoPublisher<TObject, TObject2MessageTraits>
{
protected:
    template <typename... TFields>
    using TCombinedWithoutObject = TCombinedParser<TObject, TFields...>;

    template <typename... TFields>
    using TFieldListImpl = typename TCombinedList<TCombinedWithoutObject, TParserGlue, TTypeList<>, TFields...>::TResult;

public:
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::ParseFromStringBuf;
};

// Parser and serializer
template <typename TObject, typename TObject2MessageTraits, bool useProto2Syntax>
class TBaseParserOrSerializer<TObject, TObject2MessageTraits, useProto2Syntax, true, true> :
    protected TWireTypeTags,
    public TFieldCollection<TObject, useProto2Syntax>,
    public TTraitsInfoPublisher<TObject, TObject2MessageTraits>
{
protected:
    template <typename... TFields>
    using TCombinedWithoutObject = TCombinedParserAndSerializer<TObject, TFields...>;

    template <typename... TFields>
    using TFieldListImpl = typename TCombinedList<TCombinedWithoutObject, TParserAndSerializerGlue, TTypeList<>, TFields...>::TResult;

public:
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::GetSerializedSize;
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::SerializeWithCachedSizes;
    using TTraitsInfoPublisher<TObject, TObject2MessageTraits>::ParseFromStringBuf;
};

} // namespace NInPlaceProto
