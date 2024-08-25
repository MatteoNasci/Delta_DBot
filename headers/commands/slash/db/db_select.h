#pragma once
#ifndef H_MLN_DB_DB_SELECT_H
#define H_MLN_DB_DB_SELECT_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_select : public base_db_command {
		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name;
			bool valid_stmt;
		};
		data_t data;
		
	public:
		db_select(bot_delta* const delta);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) override;
		db_init_type_flag get_requested_initialization_type(db_command_type cmd) override;

	private:
		dpp::task<void> select(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
	};
}

#endif //H_MLN_DB_DB_SELECT_H