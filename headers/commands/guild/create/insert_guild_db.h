#pragma once
#ifndef H_MLN_DB_INSERT_GUILD_DB_H
#define H_MLN_DB_INSERT_GUILD_DB_H

#include "commands/guild/create/base_guild_create.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp {
    class cluster;
    struct guild_create_t;
}

namespace mln {
    class database_handler;

    class insert_guild_db final : public base_guild_create {
    private:
        size_t saved_insert_guild_query;
        database_handler& db;
        db_saved_stmt_state db_state;
    public:
        insert_guild_db(dpp::cluster& cluster, database_handler& in_db);
        ~insert_guild_db();

        insert_guild_db(const insert_guild_db&) = delete;

        insert_guild_db(insert_guild_db&& rhs) noexcept;

        insert_guild_db& operator=(const insert_guild_db&) = delete;

        insert_guild_db& operator=(insert_guild_db&& rhs) noexcept;

        dpp::task<void> command(const dpp::guild_create_t& event_data) override final;
        std::optional<std::function<void()>> job(const dpp::guild_create_t& event_data) override final;
        bool use_job() const noexcept override final;
    };
}

#endif //H_MLN_DB_INSERT_GUILD_DB_H