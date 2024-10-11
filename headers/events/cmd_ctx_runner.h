#pragma once
#ifndef H_MLN_DB_CMD_CTX_RUNNER_H
#define H_MLN_DB_CMD_CTX_RUNNER_H

#include "commands/ctx/base_ctx_command.h"
#include "events/base_event.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace dpp {
    class cluster;
}

namespace mln {
    class jobs_runner;
    class database_handler;

    class cmd_ctx_runner final : public base_event<std::unordered_map<std::string, std::unique_ptr<base_ctx_command>>> {
    private:
        size_t event_id;
        bool initialized;
    public:
        cmd_ctx_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner);
        ~cmd_ctx_runner();
        cmd_ctx_runner(const cmd_ctx_runner&) = delete;
        cmd_ctx_runner(cmd_ctx_runner&& rhs) = delete;
        cmd_ctx_runner& operator=(const cmd_ctx_runner&) = delete;
        cmd_ctx_runner& operator=(cmd_ctx_runner&& rhs) = delete;

        void attach_event() override final;
    };
}

#endif //H_MLN_DB_CMD_CTX_RUNNER_H