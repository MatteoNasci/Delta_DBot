#pragma once
#ifndef H_MLN_DB_REGISTER_COMMANDS_H
#define H_MLN_DB_REGISTER_COMMANDS_H

#include "commands/ready/base_ready.h"

#include <dpp/coro/async.h>
#include <dpp/appcommand.h>
#include <dpp/restresults.h>

namespace mln {
    class cmd_ctx_runner;
    class cmd_runner;

    class register_commands final : public base_ready{
    private:
        
        cmd_runner& runner_cmd_ptr;
        cmd_ctx_runner& runner_ctx_ptr;
    public:
        register_commands(dpp::cluster& cluster, cmd_runner& runner_cmd_ptr, cmd_ctx_runner& runner_ctx_ptr);
        dpp::task<void> command(const dpp::ready_t& event_data) const override;

        register_commands(const register_commands&) = default;

        register_commands(register_commands&&) = default;

        register_commands& operator=(const register_commands&) = default;

        register_commands& operator=(register_commands&&) = default;
    private:

        void add_cmds_ids_to_runners(const dpp::slashcommand_map& map) const;
    };
}

#endif //H_MLN_DB_REGISTER_COMMANDS_H