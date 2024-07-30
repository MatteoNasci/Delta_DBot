cmake_minimum_required(VERSION 3.5)

file(READ "headers/version.h" ver)

string(REGEX MATCH "MLN_DB_VERSION_MAJOR ([0-9]*)" _ ${ver})
set(ver_major ${CMAKE_MATCH_1})

string(REGEX MATCH "MLN_DB_VERSION_MINOR ([0-9]*)" _ ${ver})
set(ver_minor ${CMAKE_MATCH_1})

string(REGEX MATCH "MLN_DB_VERSION_PATCH ([0-9]*)" _ ${ver})
set(ver_patch ${CMAKE_MATCH_1})

string(REGEX MATCH "MLN_DB_VERSION_TWEAK ([0-9]*)" _ ${ver})
set(ver_tweak ${CMAKE_MATCH_1})

message("version: ${ver_major}.${ver_minor}.${ver_patch}.${ver_tweak}")

project(DB VERSION ${ver_major}.${ver_minor}.${ver_patch}.${ver_tweak} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(DPP CONFIG REQUIRED)

include_directories(headers ../UnitTests/headers)

#set(libsrc source/heap/generic/heap.c source/heap/heap_int.c source/heap/heap_llint.c source/heap/heap_char.c source/heap/heap_uint.c source/heap/heap_double.c source/heap/heap_short.c source/heap/heap_float.c source/heap/heap_size_t.c)
set(testsrc test/source/main.cpp)

#add_library(MLN_Data_obj OBJECT ${libsrc})
#add_library(MLN_Data SHARED $<TARGET_OBJECTS:MLN_Data_obj>)
#add_library(MLN_Data_static STATIC $<TARGET_OBJECTS:MLN_Data_obj>)

#target_compile_definitions(MLN_Data_obj PRIVATE MLN_DATA_LIBRARY_EXPORTS)

add_executable(Test ${testsrc})

#add_dependencies(MLN_Data MLN_Data_obj)
#add_dependencies(MLN_Data_static MLN_Data_obj)
#add_dependencies(Test MLN_Data)

target_link_libraries(Test PRIVATE dpp::dpp)