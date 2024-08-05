#pragma once
#ifndef H_MLN_DB_CMD_RUNNER_H
#define H_MLN_DB_CMD_RUNNER_H

#include "bot_delta_data.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <string>

struct bot_delta_data_t;
class cmd_runner{
    private:
        std::map<std::string, std::function<dpp::task<void>(bot_delta_data_t&, const dpp::slashcommand_t&)>> events;
    public:
        cmd_runner();
        void init(bot_delta_data_t& arg);
};

#endif //H_MLN_DB_CMD_RUNNER_H