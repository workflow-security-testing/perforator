LIBRARY()
PEERDIR(
    contrib/libs/protobuf
)

SRCS(
    common.cpp
    copier_base.cpp
    field_id_macro.cpp
    field_size_macro.cpp
    inplace.cpp
    macro_for_copier.cpp
    macro_for_cpp.cpp
    macro_for_header.cpp
    macro_for_serial_header.cpp
    parser.cpp
    region_data_provider.cpp
    serialize_sizes.cpp
    serialized.cpp
    serializer_base.cpp
)

END()
