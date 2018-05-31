// Protocol Buffers - Google'
// s data interchange format
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
#include <protoc-c/c_helpers.h>

#include "go_service.h"


namespace google {
namespace protobuf {
namespace compiler {
namespace go {

ServiceGenerator::ServiceGenerator(const ServiceDescriptor* descriptor,
                                   const string& dllexport_decl)
  : descriptor_(descriptor) {
  vars_["name"] = descriptor_->name();
  vars_["fullName"] = descriptor_->full_name();
  vars_["camelName"] = c::FullNameToC(descriptor_->full_name());
  vars_["package"] = descriptor_->file()->package();
}

ServiceGenerator::~ServiceGenerator() {}

void ServiceGenerator::GenerateCFile(io::Printer* printer,
        bool last_file, std::vector<std::string> &serviceVector)
{
  int n_methods = descriptor_->method_count();
  std::map<string, string>::iterator it;
  string dName, package;

  for (int i = 0; i < n_methods; i++) {
    const MethodDescriptor *method = descriptor_->method(i);
    vars_["method"] = method->name();  
    vars_["inDesc"] = c::FullNameToC(method->input_type()->full_name());
    vars_["outDesc"] = c::FullNameToC(method->output_type()->full_name());

    printer->Print(vars_, "type $camelName$_$method$ struct {\n");
    printer->Indent();
    printer->Print(vars_,"Request\t$inDesc$\n");
    printer->Print(vars_, "Reply\t$outDesc$\n");
    printer->Outdent();
    printer->Print("}\n\n");

    printer->Print(vars_, "func (r *$camelName$_$method$) Marshal(k RpcKey) (rpc string, key, value []byte, err error) {\n");
    printer->Indent();
    printer->Print(vars_,"rpc = \"/$fullName$/$method$\"\n\n");

    printer->Print(vars_, "if key, err = json.Marshal(k); err != nil {\n");
    printer->Indent();
    printer->Print("return \"\", nil, nil, err\n");
    printer->Outdent();
    printer->Print("}\n\n");

    printer->Print(vars_, "if value, err = json.Marshal(r.Request); err != nil {\n");
    printer->Indent();
    printer->Print("return \"\", nil, nil, err\n");
    printer->Outdent();
    printer->Print("}\n\n");

    printer->Print("return rpc, key, value, nil\n");
    printer->Outdent();
    printer->Print("}\n\n");

    printer->Print(vars_, "func (r *$camelName$_$method$) Unmarshal(value []byte) (err error) {\n");
    printer->Indent();

    printer->Print(vars_, "if err = json.Unmarshal(value, r.Reply); err != nil {\n");
    printer->Indent();
    printer->Print("return err\n");
    printer->Outdent();
    printer->Print("}\n\n");

    printer->Print("return nil\n");
    printer->Outdent();
    printer->Print("}\n\n");

  }
}

}  // namespace go 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
