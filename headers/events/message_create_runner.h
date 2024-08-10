#pragma once
#ifndef H_MLN_DB_MESSAGE_CREATE_RUNNER_H
#define H_MLN_DB_MESSAGE_CREATE_RUNNER_H

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <vector>
#include <functional>

namespace mln {
    class message_create_runner {
    private:
        typedef std::function<dpp::task<void>(const dpp::message_create_t&)> action;
        std::vector<action> actions;
    public:
        message_create_runner();
        void attach_event();
    };
}

#endif //H_MLN_DB_MESSAGE_CREATE_RUNNER_H