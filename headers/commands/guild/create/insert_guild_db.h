#pragma once
#ifndef H_MLN_DB_INSERT_GUILD_DB_H
#define H_MLN_DB_INSERT_GUILD_DB_H

#include "commands/guild/create/base_guild_create.h"

namespace mln {
    class bot_delta;
    class insert_guild_db final : public base_guild_create {
    private:
        size_t saved_insert_guild_query;
        bot_delta* delta;
        bool valid_saved_insert;
    public:
        insert_guild_db(bot_delta* const delta);
        dpp::job command(std::shared_ptr<dpp::guild_create_t> event_data) override;
    };
}

#endif //H_MLN_DB_INSERT_GUILD_DB_H