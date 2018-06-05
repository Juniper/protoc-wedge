### Overview:
<p>
protoc-wedge compiler is an extension to protoc-c and can be used with the
following options:
</p>
1. avro_out: Generate Apache Avro wedge broker schema from protobuf <br/>
2. desc_out: Generate JSON descriptor table from protobuf <br/>
3. avsc_out: Generate Apache Avro schema (i.e avsc) from protobuf <br/>
4. go_out: Generate golang structs from protobuf for use with JSON <br/>


### pre-requisites
* [protobuf 3](https://github.com/google/protobuf)

### Installation:
<p> Once you download the source code from git, update the protobuf-c
submodule that is referenced using the following commands:
<p> $ git submodule init </p>
<p> $ git submodule update </p>
</p>
<br/>

<p> Now run the following commands to perform the instalation: 
<p> $ autoreconf --install </p>
<p> $ ./configure </p>
<p> $ make </p>
<p> $ sudo make install </p>
<p> On successful completion of the installation, protoc-thrift binary would
be added to /usr/local/bin </p>
</p>
<br/>

### Usage:

### JSON descriptor map generation:
<p> To generate the JSON descriptor map corresponding to a set of proto files
test1.proto, test2.proto, run the following command: 
<p> $protoc-wedge --desc_out=. test1.proto test2.proto</p>
<p> The resultant file "WedgeProtoDescTable.json" will be generated in the same
directory. This file will be used an an input for JSON
marshalling/unmarshalling and can be used by gRPC, kafka plugins
</p>
</p>

### Avsc descriptor map generation:
<p> To generate the avsc schema(s) correponding to a set of proto files
test1.proto, test2.proto, run the following command: 
<p> $protoc-wedge --avsc_out=. test1.proto test2.proto</p>
<p> An avsc file will be generated for each proto file. The proto files having
services specified (i.e) RPCs will have all the record including imports as 
a part of single avsc file. The order of imports should be maintained and the
dependant files should be generated before the proto files importing them.
For example,
if c.proto imports from b.proto which in turn imports from a.proto, then the order
of generation will be
</p>
<p> $protoc-wedge --avsc_out=. a.proto</p>
<p> $protoc-wedge --avsc_out=. a.proto b.proto</p>
<p> $protoc-wedge --avsc_out=. a.proto b.proto c.proto</p>

<p>c.avsc corresponding to c.proto will have all the information from a.proto as
well as b.proto so that the avsc file for a service has all the record information
needed. The multiple steps with invoking the compiler is due to the fact that
the avsc file generated from an imported file is required for generating the
schema of a subsequent file 
<p>

This file will be provided an an input for marshalling/unmarshalling
avro binary data to go structs and will be used by kafka plugin
</p>
</p>

### Avro descriptor map generation:
<p> To generate the Avro descriptor map correponding to a set of proto files
test1.proto, test2.proto, run the following command: 
<p> $protoc-wedge --avro_out=. test1.proto test2.proto</p>
<p> The resultant file "WedgeAvroDescTable.json" will be generated in the same
directory. This file will be provided as an input for conversion between avro
and protobuf in the wedge broker.
</p>
</p>

### Golang structs generation:
<p> To generate golang structs to execute RPCs from a  set of proto files
test1.proto, test2.proto, run the following command: 
<p> $protoc-wedge --go_out=. test1.proto test2.proto</p>
<p> The resultant file "WedgeDesc.go" will have the json marshalling information
to generate the JSON Payload for each RPC. This functionality will eventually
be extended to marshal more formarts in the future for go clients.</p>
</p>



