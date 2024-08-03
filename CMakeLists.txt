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

set(botsrc source/bot.cpp source/commands/ping.cpp 
                        source/bot_delta.cpp 
                        source/commands/pong.cpp 
                        source/commands/thinking.cpp
                        source/commands/bot_info.cpp 
                        source/ctx/high_five.cpp 
                        source/commands/file.cpp 
                        source/commands/pm.cpp 
                        source/commands/msgs_get.cpp 
                        source/commands/channel_create.cpp
                        source/commands/msg_error.cpp 
                        source/commands/image.cpp 
                        source/commands/blep.cpp 
                        source/commands/show.cpp 
                        source/commands/add_role.cpp 
                        source/commands/button.cpp
                        source/commands/button2.cpp
                        source/commands/math.cpp 
                        source/commands/pring.cpp 
                        source/commands/select.cpp 
                        source/commands/select2.cpp 
                        source/commands/select3.cpp 
                        source/commands/dialog.cpp 
                        source/events/cmd_runner.cpp
                        source/events/cmd_ctx_runner.cpp
                        source/events/form_submit_runner.cpp
                        source/events/select_click_runner.cpp)

set(testsrc test/source/main.cpp)

add_library(bot SHARED ${botsrc})

add_executable(deploy_delta ${testsrc})

file(READ ".secret.txt" sec)

string(REGEX MATCH "DISCORD_BOT_TOKEN ([^\\s]+)" matchToken ${sec})
if(matchToken)
    set(TOKEN ${CMAKE_MATCH_1})
    target_compile_definitions(bot PRIVATE MLN_DB_DISCORD_BOT_TOKEN="${TOKEN}")
    message(STATUS "Discord bot Token found.")
else()
    message(WARNING "Discord bot Token not found in .secret.txt")
endif()

file(READ ".dev_id.txt" sec_id)

string(REGEX MATCH "DEV_ID ([0-9]+)" matchDevId ${sec_id})
if(matchDevId)
    set(DEV_USER_ID ${CMAKE_MATCH_1})
    target_compile_definitions(bot PRIVATE MLN_DB_DISCORD_DEV_ID=${DEV_USER_ID})
    message(STATUS "Discord dev id found.")
else()
    message(WARNING "Discord dev id not found in .dev_id.txt")
endif()

add_dependencies(deploy_delta bot)

target_link_libraries(deploy_delta PRIVATE bot)
target_link_libraries(bot PRIVATE dpp::dpp)

target_compile_definitions(bot PRIVATE MLN_BOT_LIBRARY_EXPORTS)

set_target_properties(bot PROPERTIES
    VERSION ${ver_major}.${ver_minor}.${ver_patch}.${ver_tweak}
    SOVERSION ${ver_major}
)