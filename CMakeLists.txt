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

set(dpp_path "${libs_path}/DPP/out/build/x64-MSVC-Release-Coro")
set(dpp_headers_path "${libs_path}/DPP/include")
set(dpp_dlls_path "${dpp_path}/library")
set(dpp_libs_path "${dpp_path}/library")

#set(sodium_path "${libs_path}/DPP/win32")
#set(sodium_headers_path "${sodium_path}/include")
#set(sodium_libs_path "${sodium_path}/lib")

set(SQLITE_NAME sqlite3)
find_library(SQLITE_LIB
  NAMES ${SQLITE_NAME}
  PATHS  "${sqlite_libs_path}/"
  NO_DEFAULT_PATH
  PATH_SUFFIXES lib
)
# If the library was found, print a message and link it
if(SQLITE_LIB)
  message(STATUS "Found ${SQLITE_NAME}: ${SQLITE_LIB}")
else()
  message(FATAL_ERROR "Could not find ${SQLITE_NAME}")
endif()

#set(SODIUM_NAME libsodium)
#find_library(SODIUM_LIB
#  NAMES ${SODIUM_NAME}
#  PATHS  "${sodium_libs_path}/"
#  NO_DEFAULT_PATH
#  PATH_SUFFIXES lib
#)
# If the library was found, print a message and link it
#if(SODIUM_LIB)
#  message(STATUS "Found ${SODIUM_NAME}: ${SODIUM_LIB}")
#else()
#  message(FATAL_ERROR "Could not find ${SODIUM_NAME}")
#endif()

find_file(SQLITE_DLLS
  NAMES sqlite3.dll
  PATHS "${sqlite_dlls_path}/"
  NO_DEFAULT_PATH
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
  NO_DEFAULT_PATH
  PATH_SUFFIXES lib
)
# If the library was found, print a message and link it
if(DPP_LIB)
  message(STATUS "Found ${DPP_NAME}: ${DPP_LIB}")
else()
  message(FATAL_ERROR "Could not find ${DPP_NAME}")
endif()

set(DPP_DLLS_NAMES dpp.dll opus.dll zlib1.dll libsodium.dll libssl-1_1-x64.dll libcrypto-1_1-x64.dll)
find_path(DPP_DLLS_PATH
  NAMES ${DPP_DLLS_NAMES}
  PATHS "${dpp_dlls_path}/"
  NO_DEFAULT_PATH
)
if(DPP_DLLS_PATH)
  message(STATUS "Found ${DPP_NAME} dlls path: ${DPP_DLLS_PATH}")
else()
  message(FATAL_ERROR "Could not find dlls path for ${DPP_NAME}")
endif()

foreach(DPP_DLL_NAME ${DPP_DLLS_NAMES})
  find_file(DPP_DLL_NAME_FOUND
    NAMES ${DPP_DLL_NAME}
    PATHS "${DPP_DLLS_PATH}"
    NO_DEFAULT_PATH
  )
  if(DPP_DLL_NAME_FOUND)
    set(DPP_DLLS_LIST ${DPP_DLLS_LIST} ${DPP_DLL_NAME_FOUND})
  else()
    message(FATAL_ERROR "Could not find ${DPP_DLL_NAME} for ${DPP_NAME}")
  endif()
  unset(DPP_DLL_NAME_FOUND CACHE)
endforeach()

message(STATUS "Found ${DPP_NAME} dlls: ${DPP_DLLS_LIST}")

set(botsrc  source/commands/slash/ping.cpp
            source/bot_delta.cpp
            source/commands/slash/help.cpp
            source/commands/slash/add_emoji.cpp
            source/commands/slash/avatar.cpp
            source/commands/slash/bot_info.cpp
            source/commands/slash/db/db.cpp
            source/commands/slash/pm.cpp
            source/commands/slash/add_role.cpp
            source/events/cmd_runner.cpp
            source/events/cmd_ctx_runner.cpp
            source/main.cpp
            source/utility/utility.cpp
            source/events/ready_runner.cpp
            source/commands/ready/register_commands.cpp
            source/utility/constants.cpp
            source/commands/slash/report.cpp
            source/database/database_handler.cpp
            source/events/guild_create_runner.cpp
            source/commands/guild/create/insert_guild_db.cpp
            source/commands/slash/base_slashcommand.cpp
            source/commands/ctx/base_ctx_command.cpp
            source/commands/ready/base_ready.cpp
            source/commands/guild/create/base_guild_create.cpp
            source/commands/slash/db/base_db_command.cpp
            source/commands/slash/db/db_config.cpp
            source/commands/slash/db/db_insert.cpp
            source/commands/slash/db/db_select.cpp
            source/commands/slash/db/db_show.cpp
            source/utility/caches.cpp
            source/utility/perms.cpp
            source/commands/slash/db/db_delete.cpp
            source/commands/slash/db/db_update.cpp
            source/commands/slash/db/db_help.cpp
            source/commands/slash/db/db_privacy.cpp
            source/commands/slash/changelog.cpp
            source/utility/json_err.cpp
            source/utility/http_err.cpp
            source/utility/response.cpp
            source/security/crypt.cpp
            source/commands/slash/db/db_command_type.cpp
            source/threads/jobs_runner.cpp
            source/utility/event_data_lite.cpp
            source/utility/logs.cpp
            source/time/time.cpp
            source/commands/slash/mog/arma/create_team.cpp
            source/commands/slash/mog/mog.cpp
            source/commands/slash/mog/command_type.cpp
            source/time/chronometer.cpp
            source/commands/slash/mog/base_mog_command.cpp)

add_executable(deploy_delta ${botsrc})
add_executable(deploy_delta_dev ${botsrc})

target_link_libraries(deploy_delta PRIVATE ${DPP_LIB})
target_link_libraries(deploy_delta_dev PRIVATE ${DPP_LIB})
target_link_libraries(deploy_delta PRIVATE ${SQLITE_LIB})
target_link_libraries(deploy_delta_dev PRIVATE ${SQLITE_LIB})
#target_link_libraries(deploy_delta PRIVATE ${SODIUM_LIB})
#target_link_libraries(deploy_delta_dev PRIVATE ${SODIUM_LIB})

target_compile_definitions(deploy_delta PRIVATE DPP_CORO)
target_compile_definitions(deploy_delta_dev PRIVATE DPP_CORO)
target_compile_definitions(deploy_delta_dev PRIVATE MLN_DB_DEV_BUILD)

target_include_directories(deploy_delta PRIVATE "${PROJECT_SOURCE_DIR}/headers/")
target_include_directories(deploy_delta_dev PRIVATE "${PROJECT_SOURCE_DIR}/headers/")

find_path(DPP_INCLUDES
  NAMES dpp/dpp.h
  PATHS "${dpp_headers_path}/"
)
if(DPP_INCLUDE_DIR)
  target_include_directories(deploy_delta PRIVATE ${DPP_INCLUDES})
  target_include_directories(deploy_delta_dev PRIVATE ${DPP_INCLUDES})
  message(STATUS "Found ${DPP_NAME} includes: ${DPP_INCLUDES}")
else()
  message(FATAL_ERROR "Could not find include directory for ${DPP_NAME}")
endif()

#find_path(SODIUM_INCLUDES
#  NAMES sodium/core.h
#  PATHS "${sodium_headers_path}/"
#)
#if(SODIUM_INCLUDES)
#  target_include_directories(deploy_delta PRIVATE ${SODIUM_INCLUDES})
#  target_include_directories(deploy_delta_dev PRIVATE ${SODIUM_INCLUDES})
#  message(STATUS "Found ${SODIUM_NAME} includes: ${SODIUM_INCLUDES}")
#else()
#  message(FATAL_ERROR "Could not find include directory for ${SODIUM_NAME}")
#endif()

find_path(SQLITE_INCLUDE_DIR
  NAMES sqlite3.h
  PATHS "${sqlite_headers_path}/"
)
if(SQLITE_INCLUDE_DIR)
  target_include_directories(deploy_delta PRIVATE ${SQLITE_INCLUDE_DIR})
  target_include_directories(deploy_delta_dev PRIVATE ${SQLITE_INCLUDE_DIR})
  message(STATUS "Found ${SQLITE_NAME} includes: ${SQLITE_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "Could not find include directory for ${SQLITE_NAME}")
endif()

# Loop over the DLLs and add a custom command to copy them to the output directory
set(DLLS_TO_COPY ${DPP_DLLS_LIST} ${SQLITE_DLLS})
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

# Loop over the DLLs and add a custom command to copy them to the output directory
set(DLLS_TO_COPY ${DPP_DLLS_LIST} ${SQLITE_DLLS})
foreach(DLL ${DLLS_TO_COPY})
    add_custom_command(TARGET deploy_delta_dev POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL}
        $<TARGET_FILE_DIR:deploy_delta_dev>)
endforeach()

set_target_properties(deploy_delta_dev PROPERTIES
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

file(READ ".dev_secret.txt" sec)

string(REGEX MATCH "DISCORD_BOT_TOKEN ([^\\s]+)" matchToken ${sec})
if(matchToken)
    set(TOKEN ${CMAKE_MATCH_1})
    target_compile_definitions(deploy_delta_dev PRIVATE MLN_DB_DISCORD_BOT_TOKEN="${TOKEN}")
    message(STATUS "Discord bot-dev Token found.")
else()
    message(WARNING "Discord bot-dev Token not found in .dev_secret.txt")
endif()

file(READ ".dev_id.txt" sec_id)

string(REGEX MATCH "DEV_ID ([0-9]+)" matchDevId ${sec_id})
if(matchDevId)
    set(DEV_USER_ID ${CMAKE_MATCH_1})
    target_compile_definitions(deploy_delta PRIVATE MLN_DB_DISCORD_DEV_ID=${DEV_USER_ID})
    target_compile_definitions(deploy_delta_dev PRIVATE MLN_DB_DISCORD_DEV_ID=${DEV_USER_ID})
    message(STATUS "Discord dev id found.")
else()
    message(WARNING "Discord dev id not found in .dev_id.txt")
endif()