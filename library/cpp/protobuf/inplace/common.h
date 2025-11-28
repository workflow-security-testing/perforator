#pragma once

#include "parser.h"
#include "region_data_provider.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite.h>
#include <contrib/libs/protobuf/src/google/protobuf/stubs/common.h>

#include <util/generic/typelist.h>

namespace NInPlaceProto {

// Common typedefs
using TCodedOutputStream = google::protobuf::io::CodedOutputStream;
using TWireFormatLite = google::protobuf::internal::WireFormatLite;
using TWireType = google::protobuf::internal::WireFormatLite::WireType;
using TLightParser = TRegionDataProvider;
using THeavyParser = TInplaceParser<TLightParser>;


// Template magic

template <template <typename...> typename TTargetTemplate, template<typename, typename> typename TPairCombiner, typename TCombined, typename... TOtherArgs>
class TCombinedList;

// Only works for completely empty lists
template <template <typename...> typename TTargetTemplate, template<typename, typename> typename TPairCombiner, typename... TCombinedArgs>
class TCombinedList<TTargetTemplate, TPairCombiner, TTypeList<TCombinedArgs...>> {
public:
    using TResult = TTargetTemplate<TPairCombiner<void, void>>;
};

template <template <typename...> typename TTargetTemplate, template<typename, typename> typename TPairCombiner, typename... TCombinedArgs, typename TLastArg>
class TCombinedList<TTargetTemplate, TPairCombiner, TTypeList<TCombinedArgs...>, TLastArg> {
public:
    using TResult = TTargetTemplate<TCombinedArgs..., TPairCombiner<TLastArg, void>>;
};

template <template <typename...> typename TTargetTemplate, template<typename, typename> typename TPairCombiner, typename... TCombinedArgs, typename TArg1, typename TArg2, typename... TOtherArgs>
class TCombinedList<TTargetTemplate, TPairCombiner, TTypeList<TCombinedArgs...>, TArg1, TArg2, TOtherArgs...> {
public:
    using TResult = typename TCombinedList<TTargetTemplate, TPairCombiner, TTypeList<TCombinedArgs..., TPairCombiner<TArg1, TArg2>>, TArg2, TOtherArgs...>::TResult;
};

// Each field must contain TagId enum compile constant
template <ui32 tagId>
class TTagInfo {
public:
    enum { TagId = tagId };
};

// Force sort by tags
template <typename TField1, typename TField2>
class TCheckTagOrder : public TField1 {
public:
    static_assert(TField1::TagId < TField2::TagId, "Fields tag ids must increase");
};

template <typename TField>
class TCheckTagOrder<TField, void> : public TField {
};

} // namespace NInPlaceProto
