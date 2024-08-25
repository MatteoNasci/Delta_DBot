#pragma once
#ifndef H_MLN_DB_DB_INSERT_H
#define H_MLN_DB_DB_INSERT_H

#include "commands/slash/db/base_db_command.h"
#include "utility/url.h"

#include <dpp/restresults.h>
#include <dpp/coro/async.h>

#include <string>
#include <optional>

namespace mln {
	class bot_delta;
	class db_insert : public base_db_command {
	private:
		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name, saved_param_url, saved_param_user, saved_param_desc;
			bool valid_stmt;
		};

		data_t data;
	public:
		
		db_insert(bot_delta* const delta);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) override;
		db_init_type_flag get_requested_initialization_type(db_command_type cmd) override;
	private:

		dpp::task<void> command_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> command_file(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> command_text(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> command_help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);

		dpp::task<bool> execute_query(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& current_event, const db_cmd_data_t& cmd_data, const std::string& url, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> manage_attach_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const std::string& url, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
		dpp::task<void> manage_msg_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const msg_url_t& url_ids, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking);
	};
}

#endif //H_MLN_DB_DB_INSERT_H