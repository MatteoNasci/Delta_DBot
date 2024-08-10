#pragma once
#ifndef H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H
#define H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <vector>
#include <functional>

namespace mln {
    class msg_react_remove_runner {
    private:
        typedef std::function<dpp::task<void>(const dpp::message_reaction_remove_t&)> action;
        std::vector<action> actions;
    public:
        msg_react_remove_runner();
        void attach_event();
    };
}

#endif //H_MLN_DB_MSG_REACT_REMOVE_RUNNER_H