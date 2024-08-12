#pragma once
#ifndef H_MLN_DB_CMD_RUNNER_H
#define H_MLN_DB_CMD_RUNNER_H

#include "events/base_event.h"
#include "commands/slash/base_slashcommand.h"

#include <string>
#include <unordered_map>
#include <memory>

class mln::bot_delta;
namespace mln {
    class cmd_runner final : public base_event<std::unordered_map<std::string, std::unique_ptr<base_slashcommand>>> {
    public:
        void attach_event(bot_delta* const delta) override;
    };
}

#endif //H_MLN_DB_CMD_RUNNER_H