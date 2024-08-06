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

set(libs_path "${PROJECT_SOURCE_DIR}/ext")

set(sqlite_path "${libs_path}/sqlite/sqlite-dll-win-x64-3460000")
set(sqlite_dlls_path "${sqlite_path}/bin")
set(sqlite_libs_path "${sqlite_path}/lib")
set(sqlite_headers_path "${sqlite_path}/include")

set(dpp_path "${libs_path}/DPP/out/install/x64-MSVC-Release-Coro")
set(dpp_headers_path "${dpp_path}/include/dpp-10.0")
set(dpp_cmake_path "${dpp_path}/lib/cmake/dpp")
set(dpp_dlls_path "${dpp_path}/bin")
set(dpp_libs_path "${dpp_path}/lib/dpp-10.0")

set(SQLITE_NAME sqlite3)
find_library(SQLITE_LIB
  NAMES ${SQLITE_NAME}
  PATHS  "${sqlite_libs_path}/"
  PATH_SUFFIXES lib
)
# If the library was found, print a message and link it
if(SQLITE_LIB)
  message(STATUS "Found ${SQLITE_NAME}: ${SQLITE_LIB}")
else()
  message(FATAL_ERROR "Could not find ${SQLITE_NAME}")
endif()

find_file(SQLITE_DLLS
  NAMES sqlite3.dll
  PATHS "${sqlite_dlls_path}/"
)
if(SQLITE_DLLS)
  message(STATUS "Found ${SQLITE_NAME} dlls: ${SQLITE_DLLS}")
else()
  message(FATAL_ERROR "Could not find dlls for ${SQLITE_NAME}")
endif()


set(DPP_NAME dpp)
find_library(DPP_LIB
  NAMES ${DPP_NAME}
  PATHS  "${dpp_libs_path}/"
  PATH_SUFFIXES lib
)
# If the library was found, print a message and link it
if(DPP_LIB)
  message(STATUS "Found ${DPP_NAME}: ${DPP_LIB}")
else()
  message(FATAL_ERROR "Could not find ${DPP_NAME}")
endif()

find_path(DPP_DLLS_PATH
  NAMES dpp.dll opus.dll zlib1.dll libsodium.dll libssl-1_1-x64.dll libcrypto-1_1-x64.dll
  PATHS "${dpp_dlls_path}/"
)
if(DPP_DLLS_PATH)
  message(STATUS "Found ${DPP_NAME} dlls path: ${DPP_DLLS_PATH}")
else()
  message(FATAL_ERROR "Could not find dlls path for ${DPP_NAME}")
endif()

set(DPP_DLLS "${DPP_DLLS_PATH}/dpp.dll"
             "${DPP_DLLS_PATH}/opus.dll"
             "${DPP_DLLS_PATH}/zlib1.dll"
             "${DPP_DLLS_PATH}/libsodium.dll"
             "${DPP_DLLS_PATH}/libssl-1_1-x64.dll"
             "${DPP_DLLS_PATH}/libcrypto-1_1-x64.dll")


set(botsrc  source/commands/ping.cpp 
            source/bot_delta.cpp 
            source/commands/help.cpp 
            source/commands/add_emoji.cpp 
            source/commands/avatar.cpp 
            source/commands/bot_info.cpp 
            source/ctx/high_five.cpp 
            source/commands/db.cpp 
            source/commands/pm.cpp 
            source/commands/msgs_get.cpp 
            source/commands/add_role.cpp 
            source/events/cmd_runner.cpp
            source/events/cmd_ctx_runner.cpp
            source/events/form_submit_runner.cpp
            source/events/select_click_runner.cpp
            source/main.cpp 
            source/utility/utility.cpp 
            source/events/ready_runner.cpp 
            source/ready/register_commands.cpp
            source/events/msg_react_remove_runner.cpp 
            source/events/message_create_runner.cpp 
            source/events/button_click_runner.cpp 
            source/events/autocomplete_runner.cpp 
            source/utility/constants.cpp 
            source/commands/report.cpp)

add_executable(deploy_delta ${botsrc})

target_link_libraries(deploy_delta PRIVATE ${DPP_LIB})
target_link_libraries(deploy_delta PRIVATE ${SQLITE_LIB})

target_compile_definitions(deploy_delta PRIVATE DPP_CORO)

target_include_directories(deploy_delta PRIVATE "${PROJECT_SOURCE_DIR}/headers/")

find_path(DPP_INCLUDE_DIR
  NAMES dpp/dpp.h
  PATHS "${dpp_headers_path}/"
)
if(DPP_INCLUDE_DIR)
  target_include_directories(deploy_delta PRIVATE ${DPP_INCLUDE_DIR})
  message(STATUS "Found ${DPP_NAME} includes: ${DPP_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "Could not find include directory for ${DPP_NAME}")
endif()

find_path(SQLITE_INCLUDE_DIR
  NAMES sqlite3.h
  PATHS "${sqlite_headers_path}/"
)
if(DPP_INCLUDE_DIR)
  target_include_directories(deploy_delta PRIVATE ${SQLITE_INCLUDE_DIR})
  message(STATUS "Found ${SQLITE_NAME} includes: ${SQLITE_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "Could not find include directory for ${SQLITE_NAME}")
endif()

# Loop over the DLLs and add a custom command to copy them to the output directory
set(DLLS_TO_COPY ${DPP_DLLS} ${SQLITE_DLLS})
foreach(DLL ${DLLS_TO_COPY})
    add_custom_command(TARGET deploy_delta POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        $<TARGET_FILE_DIR:deploy_delta>)
endforeach()

set_target_properties(deploy_delta PROPERTIES
    VERSION ${ver_major}.${ver_minor}.${ver_patch}.${ver_tweak}
    SOVERSION ${ver_major}
)

file(READ ".secret.txt" sec)

string(REGEX MATCH "DISCORD_BOT_TOKEN ([^\\s]+)" matchToken ${sec})
if(matchToken)
    set(TOKEN ${CMAKE_MATCH_1})
    target_compile_definitions(deploy_delta PRIVATE MLN_DB_DISCORD_BOT_TOKEN="${TOKEN}")
    message(STATUS "Discord bot Token found.")
else()
    message(WARNING "Discord bot Token not found in .secret.txt")
endif()

file(READ ".dev_id.txt" sec_id)

string(REGEX MATCH "DEV_ID ([0-9]+)" matchDevId ${sec_id})
if(matchDevId)
    set(DEV_USER_ID ${CMAKE_MATCH_1})
    target_compile_definitions(deploy_delta PRIVATE MLN_DB_DISCORD_DEV_ID=${DEV_USER_ID})
    message(STATUS "Discord dev id found.")
else()
    message(WARNING "Discord dev id not found in .dev_id.txt")
endif()