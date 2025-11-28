#pragma once

#include <util/generic/strbuf.h>
#include <util/generic/string.h>
#include <util/generic/vector.h>
#include <util/generic/buffer.h>

#include <type_traits>

/*
 * Expected pattern usage:
 *
 * my_class.h:
 *   #include <library/cpp/protobuf/inplace/macro_for_header.h>
 *
 *   class TMyClass {
 *       <...>
 *       DECLARE_NAMED_PROTO_SERIALIZER(MySerializer);
 *   };
 *
 * client.cpp
 *   TMyClass MyClass;
 *   TString ret = MyClass.MySerializer().SerializeToString();
 *   MyClass.MySerializer().ParseFromStringBuf(str);
 */
namespace NInPlaceProto {

class TSizeStack {
private:
    TVector<size_t> Sizes;

public:
    inline void Reset() {
        Sizes.clear();
    }

    inline void PushSize(size_t size) {
        Sizes.push_back(size);
    }

    inline size_t PopSize() {
        size_t ret = Sizes.back();
        Sizes.pop_back();
        return ret;
    }
};

// Serialize frontend
template <typename TSerializableType, typename TSerializerTraits>
size_t GetSerializedSize(const TSerializableType& object, TSizeStack& stack);

template <typename TSerializableType, typename TSerializerTraits>
ui8* SerializeWithCachedSizes(const TSerializableType& object, TSizeStack& stack, ui8* data);

// Parse frontend
template <typename TSerializableType, typename TSerializerTraits>
bool ParseFromStringBuf(TSerializableType& object, TStringBuf data);

template <typename TSerializableType, typename TSerializerTraits>
class TProtoSerializer {
private:
    const TSerializableType& Object;

public:
    TProtoSerializer(const TSerializableType& object)
        : Object(object)
    {
    }

    inline void ToBuffer(TBuffer& buffer) const {
        TSizeStack stack;
        size_t size = ::NInPlaceProto::GetSerializedSize<TSerializableType, TSerializerTraits>(Object, stack);
        buffer.Resize(size);
        ui8* data = (ui8*)buffer.data();
        ui8* end = ::NInPlaceProto::SerializeWithCachedSizes<TSerializableType, TSerializerTraits>(Object, stack, data);
        Y_ASSERT(data + size == end);
    }

    inline size_t GetSize() const {
        TSizeStack stack;
        return ::NInPlaceProto::GetSerializedSize<TSerializableType, TSerializerTraits>(Object, stack);
    }

    inline TString ToString() const {
        TSizeStack stack;
        size_t size = ::NInPlaceProto::GetSerializedSize<TSerializableType, TSerializerTraits>(Object, stack);
        TString ret;
        ret.ReserveAndResize(size); // Ugly hack #1. Don't need init memory
        ui8* data = (ui8*)&ret[0]; // Ugly hack #2
        ui8* end = ::NInPlaceProto::SerializeWithCachedSizes<TSerializableType, TSerializerTraits>(Object, stack, data);
        Y_ASSERT(data + size == end);
        return ret;
    }
};

template <typename TSerializableType, typename TSerializerTraits>
class TProtoParser {
private:
    TSerializableType& Object;

public:
    TProtoParser(TSerializableType& object)
        : Object(object)
    {
    }

    inline bool FromStringBuf(TStringBuf data) {
        return ::NInPlaceProto::ParseFromStringBuf<TSerializableType, TSerializerTraits>(Object, data);
    }
};

} // NInPlaceProto

#define DECLARE_NAMED_PROTO_PARSER(NAME) \
    class T##NAME; \
    inline auto NAME() -> ::NInPlaceProto::TProtoParser<std::decay_t<decltype(*this)>, T##NAME> { \
        return {*this}; \
    }

#define DECLARE_NAMED_PROTO_SERIALIZER(NAME) \
    class T##NAME; \
    inline auto NAME() const -> ::NInPlaceProto::TProtoSerializer<std::decay_t<decltype(*this)>, T##NAME> { \
        return {*this}; \
    }

#define DECLARE_NAMED_PROTO_SERIALIZER_AND_PARSER(NAME) \
    class T##NAME; \
    inline auto NAME##Serializer() const -> ::NInPlaceProto::TProtoSerializer<std::decay_t<decltype(*this)>, T##NAME> { \
        return {*this}; \
    } \
    inline auto NAME##Parser() -> ::NInPlaceProto::TProtoParser<std::decay_t<decltype(*this)>, T##NAME> { \
        return {*this}; \
    }
