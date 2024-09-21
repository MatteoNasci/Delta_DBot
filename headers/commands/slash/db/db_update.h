#pragma once
#ifndef H_MLN_DB_DB_UPDATE_H
#define H_MLN_DB_DB_UPDATE_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class database_handler;

	class db_update : public base_db_command {
	private:
		static const std::unordered_map<mln::db_command_type, std::tuple<
			mln::db_init_type_flag,
			std::function<dpp::task<void>(const mln::db_update&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>> s_mapped_commands_info;

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name, saved_param_user, saved_param_to_update;
			bool valid_stmt;
		};
		data_t data;
		data_t data_nsfw;
		database_handler& db;
	public:
		db_update(dpp::cluster& cluster, database_handler& db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;

	private:

		dpp::task<void> description(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> nsfw(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> common(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const data_t& stmt_data) const;

		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_UPDATE_H