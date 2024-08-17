#pragma once
#ifndef H_MLN_DB_DB_INSERT_H
#define H_MLN_DB_DB_INSERT_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/url_type.h"

#include <dpp/restresults.h>
#include <dpp/coro/async.h>

#include <string>

namespace mln {
	class bot_delta;
	class db_insert : public base_db_command {
	public:
		struct task_output_t {
			dpp::snowflake created_message_id;
			std::string url;
			bool error;
		};
		struct task_input_t {
			dpp::snowflake guild_id;
			dpp::snowflake dump_channel_id;
			bot_delta* delta;
		};
		struct db_params_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_user, saved_param_name, saved_param_url, saved_param_url_type, saved_param_desc;
			bool valid_stmt;
		};

	protected:
		db_params_t data;
		std::string err_msg;

		db_insert(bot_delta* const delta, db_params_t&& in_data, const std::string& err);
	public:
		db_insert(bot_delta* const delta);
		dpp::task<void> command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type type) override;
	public:

		static dpp::task<void> exec_command(const dpp::slashcommand_t& event_data, const db_params_t& params, url_type type, bot_delta* const delta, const std::string& db_execution_empty_err_msg);
		static dpp::task<task_output_t> file_command(const dpp::slashcommand_t& event_data, const task_input_t& input, dpp::async<dpp::confirmation_callback_t>& thinking);
		static dpp::task<task_output_t> text_command(const dpp::slashcommand_t& event_data, const task_input_t& input, dpp::async<dpp::confirmation_callback_t>& thinking);
	};
}

#endif //H_MLN_DB_DB_INSERT_H