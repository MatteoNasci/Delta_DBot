#pragma once
#ifndef H_MLN_DB_FORM_SUBMIT_RUNNER_H
#define H_MLN_DB_FORM_SUBMIT_RUNNER_H

#include "bot_delta_data.h"
#include <dpp/dispatcher.h>
#include <string>

struct bot_delta_data_t;
class form_submit_runner{
    private:
        std::map<std::string, std::function<void(bot_delta_data_t&, const dpp::form_submit_t&)>> events;
    public:
        form_submit_runner();
        void init(bot_delta_data_t& arg);
};

#endif