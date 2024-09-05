#pragma once
#ifndef H_MLN_DB_DB_SHOW_H
#define H_MLN_DB_DB_SHOW_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_show : public base_db_command {
	private:
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
	public:
		db_show(bot_delta* const delta);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) override;
		db_init_type_flag get_requested_initialization_type(db_command_type cmd) override;

	private:
		dpp::task<void> all(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> user(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);

		dpp::task<void> execute_show(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, const exec_show_t& stmt_data, const std::optional<std::shared_ptr<const std::vector<std::string>>>& cached_show);
	};
}

#endif //H_MLN_DB_DB_SHOW_H