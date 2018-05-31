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
 * Copyright (c) 2018, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.pb.h>
#include <protoc-c/c_helpers.h>

#include "protobuf-c.h"
#include "go_file.h"
#include "go_message.h"
#include "go_enum.h"
#include "go_service.h"

#include <iostream>
#include <unistd.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace go {

// ===================================================================

FileGenerator::FileGenerator(const FileDescriptor* file,
                             const string& dllexport_decl)
  : file_(file),
    message_generators_(
      new scoped_ptr<MessageGenerator>[file->message_type_count()]),
    enum_generators_(
      new scoped_ptr<EnumGenerator>[file->enum_type_count()]),
    service_generators_(
      new scoped_ptr<ServiceGenerator>[file->service_count()]) {

  for (int i = 0; i < file->message_type_count(); i++) {
    message_generators_[i].reset(
      new MessageGenerator(file->message_type(i), dllexport_decl));
  }

  for (int i = 0; i < file->enum_type_count(); i++) {
    enum_generators_[i].reset(
      new EnumGenerator(file->enum_type(i), dllexport_decl));
  }

  for (int i = 0; i < file->service_count(); i++) {
    service_generators_[i].reset(
      new ServiceGenerator(file->service(i), dllexport_decl));
  }
}

FileGenerator::~FileGenerator() {}

void FileGenerator::GenerateGO (io::Printer* Printer, bool create_map,
        int &map_obj_count, bool emit_endif)
{
    static std::vector<std::string> serviceVector;
    bool last_file = false;
    std::string imports;

    if (create_map) {
        Printer->Print("package WedgeClient\n\n");

        Printer->Print("import (\n");
        Printer->Indent();
        Printer->Print("\"encoding/json\"\n");
        Printer->Outdent();
        Printer->Print(")\n\n");


        Printer->Print("type RpcMetadata struct {\n");
        Printer->Indent();
        Printer->Print("Key string\n");
        Printer->Print("Value string\n");
        Printer->Outdent();
        Printer->Print("}\n\n");

        Printer->Print("type RpcKey struct {\n");
        Printer->Indent();
        Printer->Print("ClientId\tstring  //Client id of the transaction\n");
        Printer->Print("BrokerId\tstring //String to uniquely identify the broker that should process the message\n");
        Printer->Print("TransactionId\tstring //ID to uniquely identify this transaction\n");
        Printer->Print("RpcId\tstring //ID to uniquely identify an RPC invokcation in this transaction\n");
        Printer->Print("IpAddress\t[]string //IP addresses of the devices on which the RPC should be invoked\n");
        Printer->Print("Port\tstring //Port number of the routers\n");
        Printer->Print("Metadata\t[]RpcMetadata `json:\",omitempty\"` //Metadata for the RPC\n");
        Printer->Outdent();
        Printer->Print("}\n\n");
    } else {
        Printer->Print("\n\n");
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
        message_generators_[i]->GenerateEnumDefinitions(Printer);
    }

    for (int i = 0; i < file_->enum_type_count(); i++) {
        enum_generators_[i]->GenerateDefinition(Printer);
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
        message_generators_[i]->GenerateMessageDescriptor(Printer, false);
    }

    cout<<"before calling service generator"<<endl;
    for (int i = 0; i < file_->service_count(); i++) {
        service_generators_[i]->GenerateCFile(Printer, last_file,
                serviceVector);
    }
}

}  // namespace go 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
