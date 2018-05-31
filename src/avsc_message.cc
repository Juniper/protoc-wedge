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

#include <algorithm>
#include <iostream>
#include <map>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/descriptor.pb.h>
#include <protoc-c/c_helpers.h>

#include "avsc_message.h"
#include "avsc_enum.h"

using namespace std;

namespace google {
namespace protobuf {
namespace compiler {
namespace avsc {

// ===================================================================

MessageGenerator::MessageGenerator(const Descriptor* descriptor,
                                   const string& dllexport_decl)
  : descriptor_(descriptor),
    dllexport_decl_(dllexport_decl),
    field_generators_(descriptor),
    nested_generators_(new scoped_ptr<MessageGenerator>[
      descriptor->nested_type_count()]),
    enum_generators_(new scoped_ptr<EnumGenerator>[
            descriptor->enum_type_count()]) {
  for (int i = 0; i < descriptor->nested_type_count(); i++) {
    nested_generators_[i].reset(
      new MessageGenerator(descriptor->nested_type(i), dllexport_decl));
  }

  for (int i = 0; i < descriptor->enum_type_count(); i++) {
    enum_generators_[i].reset(
      new EnumGenerator(descriptor->enum_type(i), dllexport_decl));
  }
}

size_t
MessageGenerator::GetEnumCount()
{
    return descriptor_->enum_type_count();
}

MessageGenerator::~MessageGenerator() {}

//Source file stuff

void MessageGenerator::
GenerateEnumDefinitions(io::Printer* printer,
        std::map<std::string, std::string> *enumMap) {
    for (int i = 0; i < descriptor_->nested_type_count(); i++) {
      nested_generators_[i]->GenerateEnumDefinitions(printer, enumMap);
    }

    for (int i = 0; i < descriptor_->enum_type_count(); i++) {
      enum_generators_[i]->GenerateDefinition(printer, enumMap);
    }
}

static int
compare_pfields_by_number (const void *a, const void *b)
{
  const FieldDescriptor *pa = *(const FieldDescriptor **)a;
  const FieldDescriptor *pb = *(const FieldDescriptor **)b;
  if (pa->number() < pb->number()) return -1;
  if (pa->number() > pb->number()) return +1;
  return 0;
}

void MessageGenerator::
GenerateMessageDescriptor(io::Printer* printer, bool subMessage,
        std::map<std::string, std::string> *enumMap) {
    map<string, string> vars;
    std::vector<string> unionList;
    std::set<std::string> msgSet, oneofSet;
    std::set<std::string>::iterator it;
    vars["classname"] = c::FullNameToC(descriptor_->full_name());
    std::string unionEntry;
    bool emitComma = false;

    for (int i = 0; i < descriptor_->nested_type_count(); i++) {
      nested_generators_[i]->GenerateMessageDescriptor(printer, true, enumMap);
    }

    int field_count = descriptor_->field_count();

    msgSet.clear();

    printer->Print(vars, "{\n");
    printer->Indent();

    printer->Print(vars, "\"type\": \"record\",\n");
    printer->Print(vars, "\"name\": \"$classname$\",\n");
    /*
     *  Walk the oneof fields and generate documentation
     *  letting the user know about the constraints
     */
    if (descriptor_->oneof_decl_count()) {
        printer->Print("\"doc\": \"");
    }

    for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
        const OneofDescriptor *oneof = descriptor_->oneof_decl(i);
         printer->Print("Only one of the following fields should be specified: ");
        for (int j = 0; j < oneof->field_count(); j++) {
            const FieldDescriptor *field = oneof->field(j);
            string fName = field->name(); 

            printer->Print("$field$", "field", fName);

            if (j < oneof->field_count() - 1) {
                printer->Print(", ");
            }
        }

        if (i < descriptor_->oneof_decl_count() - 1) {
            printer->Print(", ");
        }
    }

    if (descriptor_->oneof_decl_count()) {
        printer->Print("\",\n");
    }

    printer->Print(vars, "\"fields\": [\n");
    printer->Indent();

    if (descriptor_->field_count()) {
        const FieldDescriptor **sorted_fields = new const FieldDescriptor *[field_count];
        for (int i = 0; i < field_count; i++) {
            sorted_fields[i] = descriptor_->field(i);
        }
        qsort (sorted_fields, field_count, sizeof (const FieldDescriptor *),
               compare_pfields_by_number);

        for (int i = 0; i < field_count; i++) {
            const FieldDescriptor *field = sorted_fields[i];
            field_generators_.get(field).GenerateField(printer, enumMap);

            if (i < (field_count - 1)) {
                printer->Print(",");
            }
            printer->Print("\n");
           
        }

        delete [] sorted_fields;
    } else {
        /*
         * For empty messages, generate a placeholder field
         * for avsc
         */
        printer->Print("{\n");
        printer->Indent();
        
        printer->Print("\"name\": \"WedgePlaceholder\",\n");
        printer->Print("\"type\": \"null\"\n");
        
        printer->Outdent();
        printer->Print("}\n");
    }

    printer->Outdent();
    printer->Print(vars, "]\n");

    printer->Outdent();
    printer->Print(vars, "}");

    if (subMessage)
        printer->Print(vars, ",\n");
}
}  // namespace avsc 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
