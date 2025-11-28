#pragma once

#include "macro_for_serial_header.h"

/*
 * Expected pattern usage:
 *
 * my_class.h:
 *   #include <library/cpp/protobuf/inplace/macro_for_header.h>
 *
 *   namespace NMine {
 *   class TMyClass {
 *       <...>
 *       DECLARE_NAMED_PROTO2_SERIALIZER(MySerializer);
 *   };
 *   } // namespace NMine
 *
 *
 * my_class_serial.h (could be a part of my_class.cpp, if no clients require serialize it as field of another class)
 *   #include "my_class.h"
 *   #include <library/cpp/protobuf/inplace/macro_for_serial_header.h>
 *
 *   namespace NMine {
 *
 *   // TMyClass method definitions
 *
 *   START_DEFINE_NAMED_PROTO2_SERIALIZER_AND_PARSER(TMyClass, MyProto, TMyProtoMessage)
 *   BIND_FIELD_TO_PROTO_FIELD(ClassFieldName1, MessageFieldName1, MyProto);
 *   BIND_FIELD_TO_SCALAR_PROTO_FIELD(ClassFieldName2, uint32, MessageFieldName2);
 *   ...
 *   END_DEFINE_NAMED_PROTO_SERIALIZER(ClassFieldName1, ..., ClassFieldNameN);
 *
 *   } // namespace NMine
 *
 *
 * my_class.cpp:
 *   #include "my_class_serial.h"
 *   #include <library/cpp/protobuf/inplace/macro_for_cpp.h>
 *
 *   DEFINE_GLOBAL_NAMED_PROTO_SERIALIZER(NMine::TMyClass, MySerializer)
 */

#define DEFINE_GLOBAL_NAMED_PROTO_SERIALIZER(TYPE, NAME) \
namespace NInPlaceProto { \
template <> \
size_t GetSerializedSize<TYPE, TYPE::T##NAME>(const TYPE& object, TSizeStack& stack) { \
    return TYPE::T##NAME::GetSerializedSize(object, stack); \
} \
template <> \
ui8* SerializeWithCachedSizes<TYPE, TYPE::T##NAME>(const TYPE& object, TSizeStack& stack, ui8* data) { \
    return TYPE::T##NAME::SerializeWithCachedSizes(object, stack, data); \
} \
}

#define DEFINE_GLOBAL_NAMED_PROTO_PARSER(TYPE, NAME) \
namespace NInPlaceProto { \
template <> \
bool ParseFromStringBuf<TYPE, TYPE::T##NAME>(TYPE& object, TStringBuf data) { \
    return TYPE::T##NAME::ParseFromStringBuf(object, data); \
} \
}

#define DEFINE_GLOBAL_NAMED_PROTO_SERIALIZER_AND_PARSER(TYPE, NAME) \
    DEFINE_GLOBAL_NAMED_PROTO_SERIALIZER(TYPE, NAME) \
    DEFINE_GLOBAL_NAMED_PROTO_PARSER(TYPE, NAME)
