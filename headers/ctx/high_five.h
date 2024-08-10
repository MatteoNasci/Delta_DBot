#pragma once
#ifndef H_MLN_DB_HIGH_FIVE_H
#define H_MLN_DB_HIGH_FIVE_H

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>

namespace mln {
    class high_five {
    public:
        static dpp::task<void> ctx_command(const dpp::user_context_menu_t& event_data);
        static dpp::slashcommand get_command();
        static std::string get_command_name();
    };
}

#endif //H_MLN_DB_HIGH_FIVE_H