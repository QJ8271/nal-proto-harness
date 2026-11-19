cmake_minimum_required(VERSION 3.20)
project(nal_proto_harness CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_ASAN "Enable LibFuzzer, ASan, and UBSan" ON)

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g -O1)
  add_link_options(-fsanitize=fuzzer,address,undefined)
endif()

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/h264_nal_stream.proto)

add_executable(h264_nal_fuzzer
  src/harness.cc
  ${PROTO_SRCS}
  ${PROTO_HDRS}
)

target_include_directories(h264_nal_fuzzer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(h264_nal_fuzzer PRIVATE protobuf::libprotobuf protobuf-mutator-libfuzzer protobuf-mutator)
