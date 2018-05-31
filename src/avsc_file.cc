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

#include <iostream>
#include <fstream>
#include <unistd.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/descriptor.pb.h>
#include <protoc-c/c_helpers.h>

#include "protobuf-c.h"
#include "avsc_file.h"
#include "avsc_enum.h"
#include "avsc_message.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace avsc {

// ===================================================================

FileGenerator::FileGenerator(const FileDescriptor* file,
                             const string& dllexport_decl)
  : file_(file),
    message_generators_(
      new scoped_ptr<MessageGenerator>[file->message_type_count()]),
    enum_generators_(
      new scoped_ptr<EnumGenerator>[file->enum_type_count()]) {

  for (int i = 0; i < file->message_type_count(); i++) {
    message_generators_[i].reset(
      new MessageGenerator(file->message_type(i), dllexport_decl));
  }
 
  for (int i = 0; i < file->enum_type_count(); i++) {
    enum_generators_[i].reset(
      new EnumGenerator(file->enum_type(i), dllexport_decl));
  }

  c::SplitStringUsing(file_->package(), ".", &package_parts_);
}

FileGenerator::~FileGenerator() {}

/*
 * Method to determine the final set of files
 * whose contents need to be printed for this
 * avsc file
 */
void
FileGenerator::getImportAVSCList (std::map<std::string,
        std::set<std::string> > *importMap, std::set<string> *finalList)
{
    std::set<string> importSet, tempSet, intersect;
    std::map<std::string, std::set<std::string> >::iterator it;
    std::set<std::string>::iterator sit, temp;


    for (int i = 0; i < file_->dependency_count(); i++) {
        string dependName = c::StripProto(file_->dependency(i)->name());
        it = importMap->find(dependName);

        /*
         * if the import is not found, then the order of files passed
         * to the compiler is incorrect. Abort
         */
        if (it == importMap->end()) {
            std::cout<<"Import map doesn't have "<<dependName<<
                ", Incorrect order of files passed to the compiler" <<endl;
            assert(0);
        }

        importSet = it->second; 

        /*
         * Check if this entry already imports from an avsc file
         * that was added earlier to finalList. If so, then add this
         * file entry and remove the previous one so as to avoid
         * duplicate entries
         */
        for (sit = importSet.begin();  sit != importSet.end(); sit++) {
            if (finalList->find(*sit) != finalList->end()) {
                finalList->erase(*sit);
            }
        }

        finalList->insert(dependName);
    }

    /*
     * Finally check for diamond inheritance problem
     * where A is imported by Both B,C and D then imports
     * from B and C as this will also cause duplicate entries
     */
    for (sit = finalList->begin(); sit != finalList->end(); sit++) {
        it = importMap->find(*sit);
        if (it == importMap->end()) {
            std::cout<<"Import map doesn't have "<<*sit<<endl;
            assert(0);

        }
        importSet = it->second;
        temp = sit;

        for (temp++; temp != finalList->end(); temp++) {
            it = importMap->find(*temp);

            if (it == importMap->end()) {
                std::cout<<"Import map doesn't have "<<*temp<<endl;
                assert(0);
            }
            tempSet = it->second;

            std::set_intersection(importSet.begin(), importSet.end(),
                    tempSet.begin(), tempSet.end(), std::inserter(intersect,
                    intersect.begin()));

            /*
             * If the resultant set is not empty, then there are
             * common entries which would cause duplicate avsc record
             * entries and so we need to abort.
             */
            if (intersect.size()) {
                std::cout<<*sit<<" and "<<*temp<<" have common imports and will"
                    "cause duplicate entries in the resultant avsc schema"<<endl;
                assert(0);
            }
        }
    }
}

void FileGenerator::GenerateAVSC (io::Printer* printer,
        std::map<std::string, std::string> *enumMap,
        std::map<std::string, std::set<std::string> > *importMap)
{
    static std::vector<std::string> serviceVector;
    bool last_file = false;
    bool genEnum = false;
    std::map<string, string> dependencyMap;
    std::set<std::string> finalList;
    std::set<std::string>::iterator sit;

    printer->Print("[\n");
    printer->Indent();

    getImportAVSCList(importMap, &finalList);

    for (sit = finalList.begin();  sit != finalList.end(); sit++) {
      string dependName = c::StripProto(*sit) + ".avsc";
      std::ifstream ifs(dependName.c_str());

      /*
       * Print the contents of dependent avsc files
       */
      if (ifs.is_open()) {
          std::string temp((std::istreambuf_iterator<char>(ifs)),
                  (std::istreambuf_iterator<char>()));

          /*
           * Remove the leading and trailing '[' and ']'
           * characters
           */
          size_t loc;
          if ((loc = temp.find("[")) == std::string::npos) {
              std::cout<<"AVSC schema doesn't start with ["<<endl;
              assert(0);
          }
          temp.erase(loc, 1);

          if ((loc = temp.rfind("]")) == std::string::npos) {
              std::cout<<"AVSC schema doesn't end with ]"<<endl;
              assert(0);
          }
          temp.erase(loc, 1);

          printer->Print("$temp$,\n", "temp", temp);
          
          ifs.close();
      }
    }

    /*
     * Add this file and its dependencies to importMap
     */
    importMap->insert(std::pair<std::string,
            std::set<std::string> >(c::StripProto(file_->name()), finalList));

    for (int i = 0; i < file_->dependency_count(); i++) {
      /*
       * Get the enum and message names imported from other files
       * and build the dependency map
       */
      string dependName = c::StripProto(file_->dependency(i)->name());

      for (int j = 0; j < file_->dependency(i)->message_type_count(); j++) {
          string key =
              c::FullNameToC(file_->dependency(i)->message_type(j)->full_name());
          string value = dependName + "." + key;
          dependencyMap[key] = value;
      }

      for (int j = 0; j < file_->dependency(i)->enum_type_count(); j++) {
          string key =
              c::FullNameToC(file_->dependency(i)->enum_type(j)->full_name());
          string value = dependName + "." + key;
          dependencyMap[key] = value;
      }
    }

    /*
     * Print record type definitions
     */
    for (int i = 0; i < file_->message_type_count(); i++) {
         message_generators_[i]->GenerateEnumDefinitions(printer, enumMap);
    }

    for (int i = 0; i < file_->enum_type_count(); i++) {
        enum_generators_[i]->GenerateDefinition(printer, enumMap);
    }

    for (int i = 0; i < file_->message_type_count(); i++) {
        message_generators_[i]->GenerateMessageDescriptor(printer, false,
                enumMap);
        
        if (i != file_->message_type_count() - 1) {
            printer->Print(",\n");
        } else {
            printer->Print("\n");
        }
    }

    printer->Outdent();
    printer->Print("]\n");
}

}  // namespace avsc 
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
