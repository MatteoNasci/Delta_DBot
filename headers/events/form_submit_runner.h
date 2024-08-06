#pragma once
#ifndef H_MLN_DB_FORM_SUBMIT_RUNNER_H
#define H_MLN_DB_FORM_SUBMIT_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <unordered_map>

namespace mln {
    class form_submit_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::form_submit_t&)> action;
        std::unordered_map<std::string, action> actions;
    public:
        form_submit_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_FORM_SUBMIT_RUNNER_H