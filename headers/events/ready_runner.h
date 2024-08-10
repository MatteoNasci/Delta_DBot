#pragma once
#ifndef H_MLN_DB_READY_RUNNER_H
#define H_MLN_DB_READY_RUNNER_H

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <vector>
#include <functional>

namespace mln {
    class ready_runner {
    private:
        typedef std::function<bool()> ready_condition;
        typedef std::function<dpp::task<void>(const dpp::ready_t&)> ready_action;
        std::vector<std::pair<ready_condition, ready_action>> actions;
    public:
        ready_runner();
        void attach_event();
    };
}

#endif //H_MLN_DB_READY_RUNNER_H