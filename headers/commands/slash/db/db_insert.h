#pragma once
#ifndef H_MLN_DB_DB_INSERT_H
#define H_MLN_DB_DB_INSERT_H

#include "commands/slash/db/base_db_command.h"
#include "utility/url.h"

#include <string>

namespace mln {
	class database_handler;

	class db_insert : public base_db_command {
	private:
		static const std::unordered_map<mln::db_command_type, std::tuple<
			mln::db_init_type_flag,
			std::function<dpp::task<void>(const mln::db_insert&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&, bool)>>> s_mapped_commands_info;

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name, saved_param_url, saved_param_user, saved_param_desc, saved_param_nsfw;
			bool valid_stmt;
		};

		data_t data;
		database_handler& db;
	public:
		
		db_insert(dpp::cluster& cluster, database_handler& db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;
	private:

		dpp::task<void> command_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const;
		dpp::task<void> command_file(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const;
		dpp::task<void> command_text(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const;
		dpp::task<void> command_help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const;

		dpp::task<bool> execute_query(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& current_event, const db_cmd_data_t& cmd_data, const std::string& url, bool nsfw) const;
		dpp::task<void> manage_attach_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const std::tuple<std::string, std::string>& url_name, bool nsfw) const;
		dpp::task<void> manage_msg_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const msg_url_t& url_ids, bool nsfw) const;
	};
}

#endif //H_MLN_DB_DB_INSERT_H