#pragma once
#ifndef H_MLN_DB_DB_H
#define H_MLN_DB_DB_H

#include "commands/slash/base_slashcommand.h"
#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"

#include <dpp/coro/job.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class database_handler;

    class db final : public base_slashcommand {
    private:
        std::array<std::unique_ptr<mln::base_db_command>, 8> commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_insert_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_update_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_delete_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_config_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_select_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_show_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_help_sub_commands;
        const std::unordered_map<std::string, std::tuple<size_t, db_command_type>> allowed_privacy_sub_commands;
        const std::unordered_map<std::string, const std::unordered_map<std::string, std::tuple<size_t, db_command_type>>&> allowed_primary_sub_commands;

        database_handler& database;
    public:
        db(dpp::cluster& cluster, database_handler& database);
        dpp::job command(dpp::slashcommand_t event_data) const override final;

        std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) const override final;
        bool use_job() const override final;
    };
}

#endif //H_MLN_DB_DB_H