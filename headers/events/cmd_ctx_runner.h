#pragma once
#ifndef H_MLN_DB_CMD_CTX_RUNNER_H
#define H_MLN_DB_CMD_CTX_RUNNER_H

#include "events/base_event.h"
#include "commands/ctx/base_ctx_command.h"

#include <string>
#include <unordered_map>
#include <memory>

namespace mln {
    class cmd_ctx_runner final : public base_event<std::unordered_map<std::string, std::unique_ptr<base_ctx_command>>> {
    private:
        size_t event_id;
        std::atomic_bool initialized;
        std::unordered_map<size_t, std::unique_ptr<base_ctx_command>> id_to_cmd_map;
    public:
        cmd_ctx_runner(dpp::cluster& cluster, database_handler& db);
        void attach_event() override;
        void add_command_ids(const dpp::slashcommand_map& map);
    };
}

#endif //H_MLN_DB_CMD_CTX_RUNNER_H