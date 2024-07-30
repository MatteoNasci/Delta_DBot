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

set(testsrc source/main.cpp)

add_executable(Test ${testsrc})

file(READ ".secret.txt" sec)

string(REGEX MATCH "DISCORD_BOT_TOKEN ([^\\s]+)" match ${sec})
if(match)
    set(TOKEN ${CMAKE_MATCH_1})
    target_compile_definitions(Test PRIVATE MLN_DB_DISCORD_BOT_TOKEN="${TOKEN}")
    message(STATUS "Discord Bot Token found.")
else()
    message(WARNING "Discord Bot Token not found in .secret.txt")
endif()

target_link_libraries(Test PRIVATE dpp::dpp)