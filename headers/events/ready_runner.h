#pragma once
#ifndef H_MLN_DB_READY_RUNNER_H
#define H_MLN_DB_READY_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <utility>

namespace mln {
    class ready_runner {
    private:
        typedef std::function<bool(bot_delta_data_t&)> ready_condition;
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::ready_t&)> ready_action;
        std::vector<std::pair<ready_condition, ready_action>> actions;
    public:
        ready_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_READY_RUNNER_H