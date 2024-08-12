#pragma once
#ifndef H_MLN_DB_SELECT_CLICK_RUNNER_H
#define H_MLN_DB_SELECT_CLICK_RUNNER_H

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <unordered_map>
#include <functional>

namespace mln {
    class select_click_runner {
    private:
        typedef std::function<dpp::task<void>(const dpp::select_click_t&)> action;
        std::unordered_map<std::string, action> actions;
    public:
        select_click_runner();
        void attach_event();
    };
}

#endif //H_MLN_DB_SELECT_CLICK_RUNNER_H