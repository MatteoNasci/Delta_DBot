#pragma once
#ifndef H_MLN_DB_THINKING_H
#define H_MLN_DB_THINKING_H

#include "bot_delta_data.h"

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>
#include <dpp/cluster.h>

#include <string>

class thinking{
    public:
        static dpp::task<void> command(bot_delta_data_t& data, const dpp::slashcommand_t& event_data);
        static dpp::slashcommand get_command(dpp::cluster& bot);
        static std::string get_command_name();
};

#endif //H_MLN_DB_THINKING_H