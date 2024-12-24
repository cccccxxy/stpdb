cmake_minimum_required(VERSION 3.10)
project(LsmdbClient)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(leveldb REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(Protobuf_INCLUDE_DIRS /usr/local/protobuf/include)
set(Protobuf_LIBRARIES /usr/local/protobuf/lib/libprotobuf.so)

add_executable(lsmdb_client main.cc lsmdb.pb.cc lsmdb.grpc.pb.cc)

target_include_directories(lsmdb_client PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(lsmdb_client PRIVATE ${Protobuf_LIBRARIES} gRPC::grpc++ gRPC::grpc leveldb)
