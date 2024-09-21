#pragma once
#ifndef H_MLN_DB_CMD_RUNNER_H
#define H_MLN_DB_CMD_RUNNER_H

#include "events/base_event.h"
#include "commands/slash/base_slashcommand.h"

#include <string>
#include <unordered_map>
#include <atomic>

namespace mln {
    class cmd_runner final : public base_event<std::unordered_map<std::string, std::unique_ptr<base_slashcommand>>> {
    private:
        size_t event_id;
        std::atomic_bool initialized;
        std::unordered_map<size_t, std::unique_ptr<base_slashcommand>> id_to_cmd_map;
    public:
        cmd_runner(dpp::cluster& cluster, database_handler& db);
        void attach_event() override;
        void add_command_ids(const dpp::slashcommand_map& map);
    };
}

#endif //H_MLN_DB_CMD_RUNNER_H