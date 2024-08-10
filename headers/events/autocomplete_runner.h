#pragma once
#ifndef H_MLN_DB_AUTOCOMPLETE_RUNNER_H
#define H_MLN_DB_AUTOCOMPLETE_RUNNER_H

#include <dpp/dispatcher.h>
#include <dpp/appcommand.h>
#include <dpp/coro/task.h>

#include <string>
#include <unordered_map>
#include <functional>

namespace mln {
    class autocomplete_runner {
    private:
        typedef std::function<dpp::task<void>(const dpp::autocomplete_t&, const dpp::command_option&)> action;
        std::unordered_map<std::string, action> actions;
    public:
        autocomplete_runner();
        void attach_event();
    };
}

#endif //H_MLN_DB_AUTOCOMPLETE_RUNNER_H