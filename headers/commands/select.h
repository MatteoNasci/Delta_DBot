#pragma once
#ifndef H_MLN_DB_SELECT_H
#define H_MLN_DB_SELECT_H

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include "bot_delta.h"
#include <string>


class select{
    public:
        static void select_command(bot_delta_data_t& data, const dpp::select_click_t& event_data);
        static std::string get_custom_id();
        static void command(bot_delta_data_t& data, const dpp::slashcommand_t& event_data);
        static dpp::slashcommand get_command(dpp::cluster& bot);
        static std::string get_command_name();
};


#endif