#pragma once
#ifndef H_MLN_DB_MOG_ARMA_H
#define H_MLN_DB_MOG_ARMA_H

#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "database/database_handler.h"
#include "database/db_result.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>
#include <dpp/timer.h>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	namespace mog {
		struct mog_cmd_data_t;
		class mog_team;

		typedef std::array<uint64_t, 3> arma_time_list;

		class mog_arma final : public base_mog_command {
		private:
			mln::mog::mog_team& teams_handler;

			database_handler database;

			dpp::timer db_optimize_timer;
			int insert_guild_param, insert_time_param;
			size_t saved_optimize_db;
			size_t saved_insert_arma_time;
			size_t saved_select_all;
			size_t saved_select;

			std::unordered_map<uint64_t, arma_time_list> guilds_arma_date;
		public:
			mog_arma(dpp::cluster& cluster, mln::mog::mog_team& teams_handler, const std::string& database_file_name);
			~mog_arma();

			mog_arma(const mog_arma&) = delete;
			mog_arma(mog_arma&&) = delete;
			mog_arma& operator=(const mog_arma&) = delete;
			mog_arma& operator=(mog_arma&&) = delete;

			dpp::task<void> command(const dpp::slashcommand_t& event_data, mln::mog::mog_cmd_data_t& cmd_data, const mln::mog::mog_command_type type) override final;
			mln::mog::mog_init_type_flag get_requested_initialization_type(const mln::mog::mog_command_type cmd) const noexcept override final;
			db_saved_stmt_state is_db_initialized() const noexcept override final;

		private:
			dpp::task<void> raw_cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> start(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data);
			dpp::task<void> scheduled(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data);
			dpp::task<void> common_cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const uint64_t cd_delay) const;
			dpp::task<void> show_cooldowns(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const;
			dpp::task<void> help(mog_cmd_data_t& cmd_data) const;

			mln::db_result_t load_guilds_arma_time();
			static uint64_t get_next_arma_time(const uint64_t current_time, const arma_time_list& times);
			static uint64_t get_next_arma_time_upd(const uint64_t current_time, arma_time_list& times, bool& updated);
		};
	}
}

#endif //H_MLN_DB_MOG_ARMA_H