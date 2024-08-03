#pragma once
#ifndef H_MLN_DB_HIGH_FIVE_H
#define H_MLN_DB_HIGH_FIVE_H

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include "bot_delta.h"
#include <string>

class high_five{
    public:
        static void ctx_command(bot_delta_data_t& data, const dpp::user_context_menu_t& event_data);
        static dpp::slashcommand get_command(dpp::cluster& bot);
        static std::string get_command_name();
};

#endif