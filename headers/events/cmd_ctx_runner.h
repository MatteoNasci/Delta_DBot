#pragma once
#ifndef H_MLN_DB_CMD_CTX_RUNNER_H
#define H_MLN_DB_CMD_CTX_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>
#include <unordered_map>

namespace mln {
    class cmd_ctx_runner {
    private:
        typedef std::function<dpp::task<void>(bot_delta_data_t&, const dpp::user_context_menu_t&)> action;
        std::unordered_map<std::string, action> actions;
    public:
        cmd_ctx_runner();
        void attach_event(bot_delta_data_t& arg);
    };
}

#endif //H_MLN_DB_CMD_CTX_RUNNER_H