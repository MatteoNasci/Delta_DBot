#pragma once
#ifndef H_MLN_DB_REGISTER_COMMANDS_H
#define H_MLN_DB_REGISTER_COMMANDS_H

#include "commands/ready/base_ready.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp {
    class cluster;
    struct ready_t;
}

namespace mln {
    class cmd_ctx_runner;
    class cmd_runner;

    class register_commands final : public base_ready{
    private:
        
        cmd_runner& runner_cmd_ptr;
        cmd_ctx_runner& runner_ctx_ptr;
    public:
        register_commands(dpp::cluster& cluster, cmd_runner& runner_cmd_ptr, cmd_ctx_runner& runner_ctx_ptr) noexcept;
        dpp::task<void> command(const dpp::ready_t& event_data) override final;
        std::optional<std::function<void()>> job(const dpp::ready_t& event_data) override final;
        bool use_job() const noexcept override final;

        register_commands(const register_commands&) noexcept = default;
        register_commands(register_commands&&) noexcept = default;
        register_commands& operator=(const register_commands&) noexcept = default;
        register_commands& operator=(register_commands&&) noexcept = default;
    };
}

#endif //H_MLN_DB_REGISTER_COMMANDS_H