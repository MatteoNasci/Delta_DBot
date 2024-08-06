#pragma once
#ifndef H_MLN_DB_SELECT_CLICK_RUNNER_H
#define H_MLN_DB_SELECT_CLICK_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>

namespace mln {
    class select_click_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::select_click_t&)> action;
        std::map<std::string, action> actions;
    public:
        select_click_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_SELECT_CLICK_RUNNER_H