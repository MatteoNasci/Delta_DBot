#pragma once
#ifndef H_MLN_DB_READY_RUNNER_H
#define H_MLN_DB_READY_RUNNER_H

#include "commands/ready/base_ready.h"
#include "events/base_event.h"

#include <memory>
#include <vector>

namespace dpp {
    class cluster;
}

namespace mln {
    class cmd_runner;
    class cmd_ctx_runner;
    class jobs_runner;
    class database_handler;

    class ready_runner final : public base_event<std::vector<std::unique_ptr<base_ready>>> {
    private:
        size_t event_id;
        bool initialized;
        cmd_runner& runner;
        cmd_ctx_runner& ctx_runner;
    public:
        ready_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner, cmd_runner& runner, cmd_ctx_runner& ctx_runner);
        ready_runner() = delete;
        ~ready_runner();
        ready_runner(const ready_runner&) = delete;
        ready_runner(ready_runner&& rhs) = delete;
        ready_runner& operator=(const ready_runner&) = delete;
        ready_runner& operator=(ready_runner&& rhs) = delete;

        void attach_event() override final;
    };
}

#endif //H_MLN_DB_READY_RUNNER_H