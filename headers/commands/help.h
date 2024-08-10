#pragma once
#ifndef H_MLN_DB_HELP_H
#define H_MLN_DB_HELP_H

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>

namespace mln {
    class help {
    public:
        static dpp::task<void> command(const dpp::slashcommand_t& event_data);
        static dpp::slashcommand get_command();
        static std::string get_command_name();
    };
}

#endif //H_MLN_DB_HELP_H