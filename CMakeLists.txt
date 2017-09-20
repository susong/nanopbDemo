cmake_minimum_required(VERSION 3.8)
project(nanopbDemo)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES
        pb.h pb_common.c pb_common.h pb_decode.c pb_decode.h pb_encode.c pb_encode.h
        common.h common.c
        MessageProto.pb.h MessageProto.pb.c
        client.c
#        server.c
        )
add_executable(nanopbDemo ${SOURCE_FILES})