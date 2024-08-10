#pragma once
#ifndef H_MLN_DB_REGISTER_COMMANDS_H
#define H_MLN_DB_REGISTER_COMMANDS_H

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

namespace mln {
    class register_commands {
    public:
        static dpp::task<void> command(const dpp::ready_t& event_data);
        static bool execute_command();
    };
}

#endif //H_MLN_DB_REGISTER_COMMANDS_H