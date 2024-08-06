#pragma once
#ifndef H_MLN_DB_MESSAGE_CREATE_RUNNER_H
#define H_MLN_DB_MESSAGE_CREATE_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <utility>

namespace mln {
    class message_create_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::message_create_t&)> action;
        std::vector<action> actions;
    public:
        message_create_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_MESSAGE_CREATE_RUNNER_H