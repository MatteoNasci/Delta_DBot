#pragma once
#ifndef H_MLN_DEFINES_H
#define H_MLN_DEFINES_H

#ifndef DPP_CORO
    #error DPP_CORO is required to be defined
#endif //DPP_CORO

#ifdef MLN_DB_DISCORD_BOT_TOKEN
    #define DISCORD_BOT_TOKEN MLN_DB_DISCORD_BOT_TOKEN
#else //MLN_DB_DISCORD_BOT_TOKEN
    #include <cstdlib>
    #define DISCORD_BOT_TOKEN std::getenv("MLN_DB_DISCORD_BOT_TOKEN")
#endif //MLN_DB_DISCORD_BOT_TOKEN

#ifdef MLN_DB_DISCORD_DEV_ID
    #define DISCORD_DEV_ID MLN_DB_DISCORD_DEV_ID
#else //MLN_DB_DISCORD_DEV_ID
    #define DISCORD_DEV_ID 0 
#endif //MLN_DB_DISCORD_DEV_ID

#endif //H_MLN_DEFINES_H