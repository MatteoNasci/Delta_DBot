#pragma once
#ifndef H_MLN_DB_MOG_MOG_H
#define H_MLN_DB_MOG_MOG_H

#include "commands/slash/base_slashcommand.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_command_type.h"

#include <dpp/coro/job.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>

namespace dpp {
    class cluster;
    struct slashcommand_t;
}

namespace mln {
    class database_handler;
	namespace mog {
		class mog final : public base_slashcommand {
        private:
            database_handler& database;

            std::array<std::unique_ptr<mln::mog::base_mog_command>, 3> commands;
            const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> allowed_team_sub_commands;
            const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> allowed_arma_sub_commands;
            const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> allowed_help_sub_commands;
            const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> allowed_privacy_sub_commands;
            const std::unordered_map<std::string, const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>>&> allowed_primary_sub_commands;
        public:
            mog(dpp::cluster& cluster, database_handler& database);
            dpp::job command(dpp::slashcommand_t event_data) override final;

            std::optional<std::function<void()>> job(dpp::slashcommand_t event_data) override final;
            bool use_job() const noexcept override final;
		};
	}
}

#endif //H_MLN_DB_MOG_MOG_H