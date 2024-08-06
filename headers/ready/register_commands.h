#pragma once
#ifndef H_MLN_DB_REGISTER_COMMANDS_H
#define H_MLN_DB_REGISTER_COMMANDS_H

#include "bot_delta_data.h"

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>

namespace mln {
    class register_commands {
    public:
        static dpp::task<void> command(bot_delta_data_t& data, const dpp::ready_t& event_data);
        static bool execute_command(bot_delta_data_t& data);
    };
}

#endif //H_MLN_DB_REGISTER_COMMANDS_H