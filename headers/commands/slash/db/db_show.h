#pragma once
#ifndef H_MLN_DB_DB_SHOW_H
#define H_MLN_DB_DB_SHOW_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class database_handler;

	class db_show : public base_db_command {
	private:
		static const std::unordered_map<mln::db_command_type, std::tuple<
			mln::db_init_type_flag,
			std::function<dpp::task<void>(const mln::db_show&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>> s_mapped_commands_info;

		struct exec_show_t {
			size_t stmt;
			uint64_t target;
		};
		struct data_t {
			size_t saved_stmt_all, saved_stmt_user;
			int saved_param_guild, saved_param_user;
			bool valid_stmt;
		};
		data_t data;
		database_handler& db;
	public:
		db_show(dpp::cluster& cluster, database_handler& db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;

	private:
		dpp::task<void> all(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> user(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;

		dpp::task<void> execute_show(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const exec_show_t& stmt_data, const std::optional<std::shared_ptr<const std::vector<std::string>>>& cached_show) const;
	};
}

#endif //H_MLN_DB_DB_SHOW_H