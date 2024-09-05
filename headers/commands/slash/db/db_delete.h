#pragma once
#ifndef H_MLN_DB_DB_DELETE_H
#define H_MLN_DB_DB_DELETE_H

#include "commands/slash/db/base_db_command.h"

namespace mln {
	class db_delete : public base_db_command {
	private:
		struct data_t {
			size_t saved_guild, saved_self, saved_user, saved_single;
			int saved_param_single_name, saved_param_single_guild, saved_param_single_user;
			int saved_param_user_guild, saved_param_user_user;
			bool valid_stmt;
		};
		data_t data;
	public:
		db_delete(bot_delta* const delta);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) override;
		db_init_type_flag get_requested_initialization_type(db_command_type cmd) override;

	private:
		dpp::task<void> single(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> guild(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> self(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> user(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);

		dpp::task<void> exec(const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, size_t stmt, uint64_t target);

		dpp::task<void> help(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
	};
}

#endif //H_MLN_DB_DB_DELETE_H