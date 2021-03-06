/*
 * $Id$
 *
 * Copyright (c) 2018, Juniper Networks, Inc.
 * All rights reserved.
 *
 */

#include <google/protobuf/compiler/command_line_interface.h>

#include "avro_generator.h"
#include "avsc_generator.h"
#include "go_generator.h"
#include "proto_desc_generator.h"

#define PACKAGE_STRING        "protobuf-c 1.1.1"

int main(int argc, char* argv[]) {
  google::protobuf::compiler::CommandLineInterface cli;

  // Support generation of Foo code.
  google::protobuf::compiler::proto_desc::PROTO_DESCGenerator proto_desc_generator;
  cli.RegisterGenerator("--desc_out", &proto_desc_generator,
          "Generate Protobuf message descriptor Map in JSON");

  google::protobuf::compiler::avro::AVROGenerator avro_generator;
  cli.RegisterGenerator("--avro_out", &avro_generator,
          "Generate Apache Avro wedge broker schema from protobuf");

  google::protobuf::compiler::avsc::AVSCGenerator avsc_generator;
  cli.RegisterGenerator("--avsc_out", &avsc_generator,
          "Generate Apache Avro schema from protobuf");

  google::protobuf::compiler::go::GOGenerator go_generator;
  cli.RegisterGenerator("--go_out", &go_generator,
          "Generate golang structs from protobuf");

  // Add version info generated by automake
  cli.SetVersionInfo(PACKAGE_STRING);
  
  return cli.Run(argc, argv);
}
