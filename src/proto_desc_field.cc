// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

// Copyright (c) 2008-2013, Dave Benson.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Modified to implement C code by Dave Benson.

/*
 * $Id$
 *
 * Copyright (c) 2017, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/printer.h>
#include <protobuf-c/protobuf-c.h>
#include <protoc-c/c_helpers.h>

#include "proto_desc_field.h"
#include "proto_desc_primitive_field.h"
#include "proto_desc_string_field.h"
#include "proto_desc_bytes_field.h"
#include "proto_desc_enum_field.h"
#include "proto_desc_message_field.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace proto_desc {

FieldGenerator::~FieldGenerator()
{
}
static bool is_packable_type(FieldDescriptor::Type type)
{
  return type == FieldDescriptor::TYPE_DOUBLE
      || type == FieldDescriptor::TYPE_FLOAT
      || type == FieldDescriptor::TYPE_INT64
      || type == FieldDescriptor::TYPE_UINT64
      || type == FieldDescriptor::TYPE_INT32
      || type == FieldDescriptor::TYPE_FIXED64
      || type == FieldDescriptor::TYPE_FIXED32
      || type == FieldDescriptor::TYPE_BOOL
      || type == FieldDescriptor::TYPE_UINT32
      || type == FieldDescriptor::TYPE_ENUM
      || type == FieldDescriptor::TYPE_SFIXED32
      || type == FieldDescriptor::TYPE_SFIXED64
      || type == FieldDescriptor::TYPE_SINT32
      || type == FieldDescriptor::TYPE_SINT64;
    //TYPE_BYTES
    //TYPE_STRING
    //TYPE_GROUP
    //TYPE_MESSAGE
}

void FieldGenerator::GenerateDescriptorInitializerGeneric(io::Printer* printer,
							  bool optional_uses_has,
							  const string &type_macro,
							  const string &descriptor_addr) const
{
  map<string, string> variables;
  variables["field_id"] = c::SimpleItoa(descriptor_->number());
  variables["label"] = c::CamelToUpper(c::GetLabelName(descriptor_->label()));
  variables["type"] = type_macro;
  variables["name"] = descriptor_->name();
  variables["descriptor_addr"] = descriptor_addr;
  const OneofDescriptor *oneof = descriptor_->containing_oneof();
  uint32_t flags = 0;

  if (!oneof) {
      variables["union_id"] = c::SimpleItoa(INVALID_UNION_ID);
  } else {
      const FieldDescriptor *desc = oneof->field(0);
      variables["union_id"] = c::SimpleItoa(desc->number());
  }

  if (oneof != NULL) {
      variables["oneOfName"] = oneof->full_name();
  } else {
      variables["oneOfName"] = "NULL";
  }

  if (descriptor_->label() == FieldDescriptor::LABEL_REPEATED
   && is_packable_type (descriptor_->type())
   && descriptor_->options().packed())
    flags |= PROTOBUF_C_FIELD_FLAG_PACKED;

  if (descriptor_->options().deprecated())
    flags |= PROTOBUF_C_FIELD_FLAG_DEPRECATED;

  if (oneof != NULL)
    flags |= PROTOBUF_C_FIELD_FLAG_ONEOF;

  variables["flags"] = c::SimpleItoa(flags);

  printer->Print(variables, "\"$field_id$\": {\n");
  printer->Indent();
  printer->Print(variables,
    "\"name\": \"$name$\",\n"      
    "\"label\": \"PROTOBUF_C_LABEL_$label$\",\n"
    "\"type\": \"PROTOBUF_C_TYPE_$type$\",\n"
    "\"flags\": $flags$,\n"
    "\"unionId\": $union_id$,\n"
    "\"subMessage\": \"$descriptor_addr$\",\n"
    "\"containingOneof\": \"$oneOfName$\"\n"
  );
  printer->Outdent();
  printer->Print("}");
}

FieldGeneratorMap::FieldGeneratorMap(const Descriptor* descriptor)
  : descriptor_(descriptor),
    field_generators_(
      new scoped_ptr<FieldGenerator>[descriptor->field_count()]) {
  // Construct all the FieldGenerators.
  for (int i = 0; i < descriptor->field_count(); i++) {
    field_generators_[i].reset(MakeGenerator(descriptor->field(i)));
  }
}

FieldGenerator* FieldGeneratorMap::MakeGenerator(const FieldDescriptor* field) {
  switch (field->type()) {
    case FieldDescriptor::TYPE_MESSAGE:
      return new MessageFieldGenerator(field);
    case FieldDescriptor::TYPE_STRING:
      return new StringFieldGenerator(field);
    case FieldDescriptor::TYPE_BYTES:
      return new BytesFieldGenerator(field);
    case FieldDescriptor::TYPE_ENUM:
      return new EnumFieldGenerator(field);
    case FieldDescriptor::TYPE_GROUP:
      return 0;			// XXX
    default:
      return new PrimitiveFieldGenerator(field);
  }
}

FieldGeneratorMap::~FieldGeneratorMap() {}

const FieldGenerator& FieldGeneratorMap::get(
    const FieldDescriptor* field) const {
  GOOGLE_CHECK_EQ(field->containing_type(), descriptor_);
  return *field_generators_[field->index()];
}

}  // namespace proto_desc 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
