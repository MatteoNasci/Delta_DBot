#pragma once
#ifndef H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H
#define H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <utility>

namespace mln {
    class msg_react_remove_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::message_reaction_remove_t&)> action;
        std::vector<action> actions;
    public:
        msg_react_remove_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H