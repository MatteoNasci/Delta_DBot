#pragma once
#ifndef H_MLN_DB_READY_RUNNER_H
#define H_MLN_DB_READY_RUNNER_H

#include "events/base_event.h"
#include "commands/ready/base_ready.h"

#include <vector>

namespace mln {
    class cmd_runner;
    class cmd_ctx_runner;

    class ready_runner final : public base_event<std::vector<std::unique_ptr<base_ready>>> {
    private:
        cmd_runner& runner;
        cmd_ctx_runner& ctx_runner;
    public:
        ready_runner(dpp::cluster& cluster, database_handler& db, cmd_runner& runner, cmd_ctx_runner& ctx_runner);
        void attach_event() override;
    };
}

#endif //H_MLN_DB_READY_RUNNER_H