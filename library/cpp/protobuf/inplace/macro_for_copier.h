#pragma once

#include "copier_base.h"

/*
 * Allows define copier which will filter or copy fields removing unnecessary parts
 * Much faster than parsing + removing + serializing again
 * Can use other copiers/filters for submessages
 *
 * // Defines copier
 * START_PROTO_COPIER(TMyBrandNewCopier, TMyProtoMessage);
 * TOGGLE_PROTO_COPY(SomeUnnecessaryField);
 * TOGGLE_PROTO_COPY(OtherUnnecessaryField);
 * END_PROTO_COPIER(SomeUnnecessaryField, OtherUnnecessaryField);
 *
 * TString copiedBinaryProto;
 * bool parsingSuccess = TMyBrandNewCopier::Apply(originalBinaryProto, copiedBinaryProto);
 * if (!parsingSuccess) { <process corrupted proto here> }
 * // If success, copiedBinaryProto will not contain SomeUnnecessaryField & OtherUnnecessaryField
 *
 * // Define filter
 * START_PROTO_FILTER(TMyBrandNewFilter, TMyProtoMessage);
 * TOGGLE_PROTO_COPY(NeedThisField);
 * TOGGLE_PROTO_COPY(AndThatField);
 * SUB_MESSAGE_COPIER(SubMessageField, TSomeSubMessageFilterOrCopier);
 * END_PROTO_COPIER(NeedThisField, AndThatField, SubMessageField);
 *
 * TString filteredBinaryProto;
 * bool parsingSuccess = TMyBrandNewFilter::Apply(originalBinaryProto, filteredBinaryProto);
 * if (!parsingSuccess) { <process corrupted proto here> }
 * // If success, filteredBinaryProto will contain only NeedThisField, AndThatField & copied/filtered parts of SubMessageField
 */

#define START_PROTO_COPIER(NAME, MESSAGE) \
class NAME : public ::NInPlaceProto::TCopier<NAME, true> { \
    using TBase = TCopier<NAME, true>; \
    using TProtoMessage = MESSAGE; \
    friend class TCopier<NAME, true>;

#define START_PROTO_FILTER(NAME, MESSAGE) \
class NAME : public TCopier<NAME, false> { \
    using TBase = TCopier<NAME, false>; \
    using TProtoMessage = MESSAGE; \
    friend class TCopier<NAME, false>;

#define TOGGLE_PROTO_COPY(MESSAGE_FIELD) \
    class MESSAGE_FIELD : \
        public ::NInPlaceProto::TTagInfo<TProtoMessage::k##MESSAGE_FIELD##FieldNumber>, \
        public TBase::TInvertedConsumer { \
    };

#define SUB_MESSAGE_COPIER(MESSAGE_FIELD, SUB_COPIER_NAME) \
    class MESSAGE_FIELD : \
        public ::NInPlaceProto::TTagInfo<TProtoMessage::k##MESSAGE_FIELD##FieldNumber>, \
        public SUB_COPIER_NAME { \
    };

#define END_PROTO_COPIER(...) \
    using TFieldList = TFieldListImpl<__VA_ARGS__>; \
};
