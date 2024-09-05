#pragma once
#ifndef H_MLN_DB_DB_UPDATE_H
#define H_MLN_DB_DB_UPDATE_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_update : public base_db_command {
	private:
		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name, saved_param_user, saved_param_desc;
			bool valid_stmt;
		};
		data_t data;
	public:
		db_update(bot_delta* const delta);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) override;
		db_init_type_flag get_requested_initialization_type(db_command_type cmd) override;

	private:

		dpp::task<void> description(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);

		dpp::task<void> help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
	};
}

#endif //H_MLN_DB_DB_UPDATE_H