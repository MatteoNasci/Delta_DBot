#pragma once
#ifndef H_MLN_DB_SELECT_CLICK_RUNNER_H
#define H_MLN_DB_SELECT_CLICK_RUNNER_H

#include "bot_delta_data.h"
#include <dpp/dispatcher.h>
#include <string>

struct bot_delta_data_t;
class select_click_runner{
    private:
        std::map<std::string, std::function<void(bot_delta_data_t&, const dpp::select_click_t&)>> events;
    public:
        select_click_runner();
        void init(bot_delta_data_t& arg);
};

#endif