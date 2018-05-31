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
 * Copyright (c) 2017, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

#include <google/protobuf/io/printer.h>
#include <protoc-c/c_helpers.h>

#include "proto_desc_service.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace proto_desc {

ServiceGenerator::ServiceGenerator(const ServiceDescriptor* descriptor,
                                   const string& dllexport_decl)
  : descriptor_(descriptor) {
  vars_["name"] = descriptor_->name();
  vars_["fullName"] = descriptor_->full_name();
  vars_["package"] = descriptor_->file()->package();

  /*
   * Adding the package name to daemon name mapping
   * in the vars_ map
   */
  vars_["routing"] = "rpd";
  vars_["firewall"] = "dfwd";
  vars_["acl"] = "dfwd";
  vars_["interface"] = "dcd";
  vars_["cos"] = "cosd";
  vars_["openconfig"] = "mgd";
  vars_["management"] = "mgd";
  vars_["authentication"] = "jsd";
  vars_["registration"] = "jsd";
  vars_["events"] = "eventd";
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
    vars_["outStreamApi"] = method->server_streaming() ? "true" : "false";
    vars_["inStreamApi"] = method->client_streaming() ? "true" : "false";

    it = vars_.find(c::ToLower(vars_.find("package")->second));
    vars_["dName"] = (it != vars_.end()) ? it->second : "";

    std::string mv, fullName;
  //  it = vars_.find("fullname");
  //  fullName = it->second;

    printer->Print("{\n");
    printer->Indent();
    printer->Print(vars_, "\"/$fullName$/$method$\": {\n");
    printer->Indent();
    printer->Print(vars_,"\"inMsgDescriptor\": \"$inDesc$\",\n");
    printer->Print(vars_, "\"outMsgDescriptor\": \"$outDesc$\",\n");
    printer->Print(vars_, "\"dName\": \"$dName$\",\n");
    printer->Print("\"target\": \"\",\n");
    printer->Print(vars_, "\"inStreamAPI\": $inStreamApi$,\n");
    printer->Print(vars_, "\"outStreamAPI\": $outStreamApi$\n");
    printer->Outdent();
    printer->Print("}\n");
    printer->Outdent();
    printer->Print("}");
    if (!(last_file && i == n_methods - 1))
        printer->Print(",");
    printer->Print("\n");
  }
}

}  // namespace proto_desc 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
