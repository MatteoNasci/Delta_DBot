#pragma once
#ifndef H_MLN_DEFINES_H
#define H_MLN_DEFINES_H

#include <cstdlib>

#ifdef MLN_BOT_LIBRARY_EXPORTS
#else
#endif

#ifdef _WIN32
    #ifdef MLN_BOT_LIBRARY_EXPORTS
        #define MLN_BOT_LIBRARY_API __declspec(dllexport)
    #else
        #define MLN_BOT_LIBRARY_API __declspec(dllimport)
    #endif
#else
    #define MLN_BOT_LIBRARY_API
#endif

#ifdef MLN_DB_DISCORD_BOT_TOKEN
    #define DISCORD_BOT_TOKEN MLN_DB_DISCORD_BOT_TOKEN
#else
    #define DISCORD_BOT_TOKEN std::getenv("CLD_DISCORD_BOT_TOKEN")
#endif

#endif