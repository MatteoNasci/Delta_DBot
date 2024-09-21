#pragma once
#ifndef H_MLN_DB_DB_CONFIG_H
#define H_MLN_DB_DB_CONFIG_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class database_handler;

	class db_config final : public base_db_command {
	private:
		static const std::unordered_map<mln::db_command_type, std::tuple<
			mln::db_init_type_flag,
			std::function<dpp::task<void>(const mln::db_config&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>> s_mapped_commands_info;

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_channel;
			bool valid_stmt;
		};
		data_t data;
		database_handler& db;
	public:
		db_config(dpp::cluster& cluster, database_handler& db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;

	private:
		dpp::task<void> update_dump(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_CONFIG_H