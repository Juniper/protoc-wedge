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

// Copyright (c) 2008-2014, Dave Benson and the protobuf-c authors.
// All rights reserved.
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

#include <iostream>
#include <unistd.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.pb.h>
#include <protoc-c/c_helpers.h>

#include "protobuf-c.h"
#include "proto_desc_file.h"
#include "proto_desc_service.h"
#include "proto_desc_message.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace proto_desc {

// ===================================================================

FileGenerator::FileGenerator(const FileDescriptor* file,
                             const string& dllexport_decl)
  : file_(file),
    message_generators_(
      new scoped_ptr<MessageGenerator>[file->message_type_count()]),
    service_generators_(
      new scoped_ptr<ServiceGenerator>[file->service_count()]) {

  for (int i = 0; i < file->message_type_count(); i++) {
    message_generators_[i].reset(
      new MessageGenerator(file->message_type(i), dllexport_decl));
  }

  for (int i = 0; i < file->service_count(); i++) {
    service_generators_[i].reset(
      new ServiceGenerator(file->service(i), dllexport_decl));
  }

  c::SplitStringUsing(file_->package(), ".", &package_parts_);
}

FileGenerator::~FileGenerator() {}

void FileGenerator::GeneratePROTO_DESC (io::Printer* printer, bool create_map,
        int &map_obj_count, bool emit_endif)
{
    static std::vector<std::string> serviceVector;
    bool last_file = false;
    std::string imports;

    for (int i = 0; i < file_->dependency_count(); i++) {
        if (file_->package().compare(file_->dependency(i)->package())) {
            if (i != 0)
                imports += ",";
            imports += file_->dependency(i)->package();
        }
    }

    if (create_map) {
        printer->Print("{\n");
        printer->Indent();
        printer->Print("\"packageTable\": [\n");
    } else {
        printer->Indent();
    }

    printer->Indent();
    printer->Print("{\n");
    printer->Indent();
    printer->Print("\"$package$\": {\n", "package", file_->package());
    printer->Indent();
    printer->Print("\"imports\": \"$imports$\",\n", "imports", imports);
    printer->Print("\"messageTable\": [\n");
    printer->Indent();
    for (int i = 0; i < file_->message_type_count(); i++) {
        message_generators_[i]->GenerateMessageDescriptor(printer, false);
        if (i != file_->message_type_count() - 1)
            printer->Print(",");
        printer->Print("\n");
    }

    printer->Outdent();
    printer->Print("],\n\n");
    printer->Print("\"rpcTable\" : [\n");
    printer->Indent();
    for (int i = 0; i < file_->service_count(); i++) {
        if (i == file_->service_count() - 1)
            last_file = true;
        service_generators_[i]->GenerateCFile(printer, last_file,
                serviceVector);
    }

    printer->Outdent();
    printer->Print("]\n");
    printer->Outdent();
    printer->Print("}\n");
    printer->Outdent();
    printer->Print("}");


    if (emit_endif) {
        printer->Print("\n");
        printer->Outdent();
        printer->Print("]\n");
        printer->Outdent();
        printer->Print("}\n");
    } else {
        printer->Print(",\n");
    }
}

}  // namespace proto_desc 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
