#pragma once
#ifndef H_MLN_DB_GUILD_CREATE_RUNNER_H
#define H_MLN_DB_GUILD_CREATE_RUNNER_H

#include "events/base_event.h"
#include "commands/guild/create/base_guild_create.h"

#include <vector>

namespace mln {
    class guild_create_runner final : public base_event<std::vector<std::unique_ptr<base_guild_create>>> {
    public:
        guild_create_runner(dpp::cluster& cluster, database_handler& db);
        void attach_event() override;
    };
}

#endif //H_MLN_DB_READY_RUNNER_H