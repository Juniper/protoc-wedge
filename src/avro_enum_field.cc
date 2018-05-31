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
 * Copyright (c) 2018, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

#include <iostream>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/wire_format.h>
#include <protoc-c/c_helpers.h>

#include "avro_enum_field.h"

using namespace std;

namespace google {
namespace protobuf {
namespace compiler {
namespace avro {

using internal::WireFormat;

// TODO(kenton):  Factor out a "SetCommonFieldVariables()" to get rid of
//   repeat code between this and the other field types.
void SetEnumVariables(const FieldDescriptor* descriptor,
                      map<string, string>* variables) {
  (*variables)["name"] = c::FieldName(descriptor);
  (*variables)["type"] = c::FullNameToC(descriptor->enum_type()->name());
}

// ===================================================================

EnumFieldGenerator::
EnumFieldGenerator(const FieldDescriptor* descriptor)
  : FieldGenerator(descriptor)
{
  SetEnumVariables(descriptor, &variables_);
}

EnumFieldGenerator::~EnumFieldGenerator() {}


void EnumFieldGenerator::GenerateField (io::Printer* printer,
                  std::map<std::string, std::string> *enumMap) const
{
  std::map<std::string, std::string>::iterator it;
  string enumKey = c::FullNameToC(descriptor_->enum_type()->full_name());
  string enumName, messageName;
  size_t loc;
  string repeated, oneOfName;
  const OneofDescriptor *oneof = descriptor_->containing_oneof();

  if (oneof != NULL) {
      oneOfName = oneof->full_name();
  } else {
      oneOfName = "NULL";
  }

  it = enumMap->find(enumKey);
  if (it != enumMap->end()) {
      std::string enumVal = it->second;
      printer->Print("\"$name$\": {\n", "name", descriptor_->name());
      printer->Indent();

      if (descriptor_->label() == FieldDescriptor::LABEL_REPEATED) {
          repeated = "true";
      } else {
          repeated = "false";
      }
     
      printer->Print("\"type\": \"enum\",\n");
      printer->Print("\"repeated\": $repeated$,\n", "repeated", repeated);
      printer->Print("\"enumValues\": \"$enumVal$\",\n", "enumVal", enumVal);
      printer->Print("\"subRecord\": \"NULL\",\n");
      printer->Print("\"containingOneof\": \"$oneOfName$\"\n", "oneOfName",
              oneOfName);
      
      printer->Outdent();
      printer->Print("}");

  } else {
      std::cout<<"Entry "<<enumKey<<" not found"<<endl;
      assert(0);
  }
}

}  // namespace avro 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
