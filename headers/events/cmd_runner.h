#pragma once
#ifndef H_MLN_DB_CMD_RUNNER_H
#define H_MLN_DB_CMD_RUNNER_H

#include "commands/slash/base_slashcommand.h"
#include "events/base_event.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace dpp {
    class cluster;
}

namespace mln {
    class database_handler;
    class jobs_runner;

    class cmd_runner final : public base_event<std::unordered_map<std::string, std::unique_ptr<base_slashcommand>>> {
    private:
        size_t event_id;
        bool initialized;
    public:
        cmd_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner);
        ~cmd_runner();
        void attach_event() override final;
    };
}

#endif //H_MLN_DB_CMD_RUNNER_H