#pragma once
#ifndef H_MLN_DB_AUTOCOMPLETE_RUNNER_H
#define H_MLN_DB_AUTOCOMPLETE_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/appcommand.h>
#include <dpp/coro/task.h>

#include <string>

namespace mln {
    class autocomplete_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::autocomplete_t&, const dpp::command_option&)> action;
        std::map<std::string, action> actions;
    public:
        autocomplete_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_AUTOCOMPLETE_RUNNER_H