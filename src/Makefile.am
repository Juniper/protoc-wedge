bin_PROGRAMS = protoc-wedge

AUTOMAKE_OPTIONS = subdir-objects

protoc_wedge_SOURCES = \
    ../external/protobuf-c/protoc-c/c_extension.cc \
    ../external/protobuf-c/protoc-c/c_helpers.cc \
    avro_bytes_field.cc \
    avro_enum.cc \
    avro_enum_field.cc \
    avro_field.cc \
    avro_file.cc \
    avro_generator.cc \
    avro_message.cc \
    avro_message_field.cc \
    avro_primitive_field.cc \
    avro_service.cc \
    avro_string_field.cc \
    avsc_bytes_field.cc \
    avsc_enum.cc \
    avsc_enum_field.cc \
    avsc_field.cc \
    avsc_file.cc \
    avsc_generator.cc \
    avsc_message.cc \
    avsc_message_field.cc \
    avsc_primitive_field.cc \
    avsc_string_field.cc \
    go_bytes_field.cc \
    go_enum.cc \
    go_enum_field.cc \
    go_field.cc \
    go_file.cc \
    go_generator.cc \
    go_message.cc \
    go_message_field.cc \
    go_primitive_field.cc \
    go_service.cc \
    go_string_field.cc \
    proto_desc_bytes_field.cc \
    proto_desc_enum_field.cc \
    proto_desc_field.cc \
    proto_desc_file.cc \
    proto_desc_generator.cc \
    proto_desc_message.cc \
    proto_desc_message_field.cc \
    proto_desc_primitive_field.cc \
    proto_desc_service.cc \
    proto_desc_string_field.cc \
    main.cc

protoc_wedge_LDADD = -lprotoc -lprotobuf

AM_CXXFLAGS = \
    -I${srcdir} \
    -I${srcdir}/../external/protobuf-c \
    -I${srcdir}/../external/protobuf-c/protobuf-c
