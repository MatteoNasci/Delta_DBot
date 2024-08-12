#pragma once
#ifndef H_MLN_DB_REGISTER_COMMANDS_H
#define H_MLN_DB_REGISTER_COMMANDS_H

#include "commands/ready/base_ready.h"

namespace mln {
    class bot_delta;
    class register_commands final : public base_ready{
    public:
        register_commands(bot_delta* const delta);
        dpp::job command(std::shared_ptr<dpp::ready_t> event_data) override;
        bool execute_command() override;
    };
}

#endif //H_MLN_DB_REGISTER_COMMANDS_H