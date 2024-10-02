#pragma once
#ifndef H_MLN_DB_GUILD_CREATE_RUNNER_H
#define H_MLN_DB_GUILD_CREATE_RUNNER_H

#include "commands/guild/create/base_guild_create.h"
#include "events/base_event.h"

#include <memory>
#include <vector>

namespace dpp {
    class cluster;
}

namespace mln {
    class database_handler;
    class jobs_runner;

    class guild_create_runner final : public base_event<std::vector<std::unique_ptr<base_guild_create>>> {
    private:
        size_t event_id;
        bool initialized;
    public:
        guild_create_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner);
        ~guild_create_runner();
        void attach_event() override final;
    };
}

#endif //H_MLN_DB_READY_RUNNER_H