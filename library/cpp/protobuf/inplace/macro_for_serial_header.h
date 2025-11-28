#pragma once

#include "macro_for_header.h"
#include "serializer_base.h"

/*
 * Expected pattern usage:
 *
 * my_class.h:
 *   #include <library/cpp/protobuf/inplace/macro_for_header.h>
 *
 *   namespace NMine {
 *   class TMyClass {
 *       <...>
 *       DECLARE_NAMED_PROTO_SERIALIZER(MySerializer);
 *   };
 *   } // namespace NMine
 *
 * my_class_serial.h
 *   #include "my_class.h"
 *   #include <library/cpp/protobuf/inplace/macro_for_serial_header.h>
 *
 *   namespace NMine {
 *
 *   // TMyClass method definitions
 *
 *   START_DEFINE_NAMED_PROTO3_SERIALIZER(TMyClass, MySerializer, TMyProtoMessage)
 *   BIND_FIELD_TO_PROTO_FIELD(ClassFieldName1, uint32, MessageFieldName1);
 *   ...
 *   BIND_FIELD_TO_PROTO_FIELD(ClassFieldNameN, TOtherMessage, MessageFieldNameN);
 *   END_DEFINE_NAMED_PROTO_SERIALIZER(ClassFieldName1, ..., ClassFieldNameN);
 *
 *   } // namespace NMine
 */
#define START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, USE_PROTO2, NEED_PARSER, NEED_SERIALIZER) \
class TYPE::T##NAME : public ::NInPlaceProto::TBaseParserOrSerializer<TYPE, TYPE::T##NAME, USE_PROTO2, NEED_PARSER, NEED_SERIALIZER> { \
private: \
    using TBase = ::NInPlaceProto::TBaseParserOrSerializer<TYPE, TYPE::T##NAME, USE_PROTO2, NEED_PARSER, NEED_SERIALIZER>; \
    friend class TTraitsInfoPublisher<TYPE, TYPE::T##NAME>; \
    using TBaseType = TYPE; \
    using TProtoMessage = MESSAGE; \
    using TParser = NInPlaceProto::TInplaceParser<NInPlaceProto::TRegionDataProvider>; \
    template <bool> \
    class TAfterInitAction { \
    public: \
        static void Do(TBaseType&) {} \
    }; \
    static TProtoMessage& forDecltypeOnly();

#define START_DEFINE_NAMED_PROTO2_SERIALIZER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, true, false, true)
#define START_DEFINE_NAMED_PROTO3_SERIALIZER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, false, false, true)
#define START_DEFINE_NAMED_PROTO2_PARSER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, true, true, false)
#define START_DEFINE_NAMED_PROTO3_PARSER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, false, true, false)
#define START_DEFINE_NAMED_PROTO2_SERIALIZER_AND_PARSER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, true, true, true)
#define START_DEFINE_NAMED_PROTO3_SERIALIZER_AND_PARSER(TYPE, NAME, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER_OR_PARSER_WITH_SYNTAX(TYPE, NAME, MESSAGE, false, true, true)

#define START_AFTER_PARSE_ACTION(OBJ) \
    template <> \
    class TAfterInitAction<true> { \
    public: \
        static void Do(TBaseType& OBJ) {

#define END_AFTER_PARSE_ACTION() \
        } \
    };

#define BIND_FIELD_TO_PROTO_FIELD_NAMED_SERIALIZER(STRUCT_FIELD, MESSAGE_FIELD, NAME) \
    class STRUCT_FIELD : public TBase::TFieldImpl< \
        ::NInPlaceProto::TAssignAction<TBaseType, decltype(TBaseType::STRUCT_FIELD), STRUCT_FIELD>, \
        TProtoMessage::k##MESSAGE_FIELD##FieldNumber, \
        ::NInPlaceProto::TClassFieldTraits<decltype(TBaseType::STRUCT_FIELD)>::TFieldType::T##NAME, \
        decltype(forDecltypeOnly().Get##MESSAGE_FIELD()), \
        false> { \
    public: \
        static inline decltype(TBaseType::STRUCT_FIELD) const& GetConstField(const TBaseType& object) { \
            return object.STRUCT_FIELD; \
        } \
        static inline decltype(TBaseType::STRUCT_FIELD)& GetMutableField(TBaseType& object) { \
            return object.STRUCT_FIELD; \
        } \
    };

#define BIND_FLATTEN_PROTO_FIELD_NAMED_SERIALIZER(STRUCT_FIELD_LIKE_NAME, MESSAGE_FIELD, NAME) \
    class STRUCT_FIELD_LIKE_NAME : public TBase::TFieldImpl< \
        ::NInPlaceProto::TAssignAction<TBaseType, TBaseType, STRUCT_FIELD_LIKE_NAME>, \
        TProtoMessage::k##MESSAGE_FIELD##FieldNumber, \
        ::NInPlaceProto::TClassFieldTraits<TBaseType>::TFieldType::T##NAME, \
        decltype(forDecltypeOnly().Get##MESSAGE_FIELD()), \
        false> { \
    public: \
        static inline TBaseType const& GetConstField(const TBaseType& object) { \
            return object; \
        } \
        static inline TBaseType& GetMutableField(TBaseType& object) { \
            return object; \
        } \
    };

#define BIND_FIELD_TO_SCALAR_PROTO_FIELD_IMPL(STRUCT_FIELD, WIRE_TYPE, MESSAGE_FIELD, PACKED) \
    class STRUCT_FIELD : public TBase::TFieldImpl< \
        ::NInPlaceProto::TAssignAction<TBaseType, decltype(TBaseType::STRUCT_FIELD), STRUCT_FIELD>, \
        TProtoMessage::k##MESSAGE_FIELD##FieldNumber, \
        TBase::T##WIRE_TYPE, \
        decltype(forDecltypeOnly().Get##MESSAGE_FIELD()), \
        PACKED> { \
    public: \
        static inline decltype(TBaseType::STRUCT_FIELD) const& GetConstField(const TBaseType& object) { \
            return object.STRUCT_FIELD; \
        } \
        static inline decltype(TBaseType::STRUCT_FIELD)& GetMutableField(TBaseType& object) { \
            return object.STRUCT_FIELD; \
        } \
    };
#define BIND_FIELD_TO_SCALAR_PROTO_FIELD(STRUCT_FIELD, WIRE_TYPE, MESSAGE_FIELD) \
    BIND_FIELD_TO_SCALAR_PROTO_FIELD_IMPL(STRUCT_FIELD, WIRE_TYPE, MESSAGE_FIELD, false)
#define BIND_FIELD_TO_PACKED_SCALAR_PROTO_FIELD(STRUCT_FIELD, WIRE_TYPE, MESSAGE_FIELD) \
    BIND_FIELD_TO_SCALAR_PROTO_FIELD_IMPL(STRUCT_FIELD, WIRE_TYPE, MESSAGE_FIELD, true)

#define BIND_ACTION_TO_SCALAR_PROTO_FIELD_IMPL(ACTION_NAME, WIRE_TYPE, MESSAGE_FIELD, PACKED) \
    class ACTION_NAME : public TBase::TFieldImpl< \
        ::NInPlaceProto::TCustomAction<TBaseType, decltype(TBaseType::STRUCT_FIELD), ACTION_NAME>, \
        TProtoMessage::k##MESSAGE_FIELD##FieldNumber, \
        TBase::T##WIRE_TYPE, \
        decltype(forDecltypeOnly().Get##MESSAGE_FIELD()), \
        PACKED>

#define BIND_ACTION_TO_SCALAR_PROTO_FIELD(ACTION_NAME, WIRE_TYPE, MESSAGE_FIELD) \
    BIND_ACTION_TO_SCALAR_PROTO_FIELD_IMPL(ACTION_NAME, WIRE_TYPE, MESSAGE_FIELD, false)
#define BIND_ACTION_TO_PACKED_SCALAR_PROTO_FIELD(ACTION_NAME, WIRE_TYPE, MESSAGE_FIELD) \
    BIND_ACTION_TO_SCALAR_PROTO_FIELD_IMPL(ACTION_NAME, WIRE_TYPE, MESSAGE_FIELD, true)


// chain part (GetSerializedSize, Serialize, ParseFromStringBuf) + endpints (SerializeToString, ParseFromStringBuf)
#define END_DEFINE_NAMED_PROTO_SERIALIZER(...) \
    using TFieldList = TFieldListImpl<__VA_ARGS__>; \
};

#define START_DEFINE_PROTO_SERIALIZER(TYPE, MESSAGE) \
    START_DEFINE_NAMED_PROTO_SERIALIZER(TYPE, DefaultProtoSerializer, MESSAGE)
#define BIND_FIELD_TO_PROTO_FIELD(STRUCT_FIELD, MESSAGE_FIELD) \
    BIND_FIELD_TO_PROTO_FIELD_NAMED_SERIALIZER(STRUCT_FIELD, MESSAGE_FIELD, DefaultProtoSerializer)
#define END_DEFINE_PROTO_SERIALIZER(TYPE) \
    END_DEFINE_NAMED_PROTO_SERIALIZER(TYPE, DefaultProtoSerializer)
