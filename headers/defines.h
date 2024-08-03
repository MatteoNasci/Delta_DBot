#pragma once
#ifndef H_MLN_DEFINES_H
#define H_MLN_DEFINES_H

#include <cstdlib>

#ifdef MLN_DB_DISCORD_BOT_TOKEN
    #define DISCORD_BOT_TOKEN MLN_DB_DISCORD_BOT_TOKEN
#else
    #define DISCORD_BOT_TOKEN std::getenv("CLD_DISCORD_BOT_TOKEN")
#endif

#ifdef MLN_DB_DISCORD_DEV_ID
    #define DISCORD_DEV_ID MLN_DB_DISCORD_DEV_ID
#else
    #define DISCORD_DEV_ID 0 
#endif

#endif