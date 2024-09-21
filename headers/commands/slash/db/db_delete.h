#pragma once
#ifndef H_MLN_DB_DB_DELETE_H
#define H_MLN_DB_DB_DELETE_H

#include "commands/slash/db/base_db_command.h"

#include <dpp/coro/async.h>
#include <dpp/timer.h>
#include <dpp/dispatcher.h>
#include <dpp/event_router.h>

namespace mln {
	class database_handler;

	class db_delete : public base_db_command {
	private:
		static const std::unordered_map<mln::db_command_type, std::tuple<
			mln::db_init_type_flag,
			std::function<dpp::task<void>(const mln::db_delete&, const dpp::slashcommand_t&, const dpp::interaction_create_t&, const mln::db_cmd_data_t&)>,
			std::string>> s_mapped_commands_info;

		struct data_t {
			size_t saved_guild, saved_self, saved_user, saved_single;
			int saved_param_single_name, saved_param_single_guild, saved_param_single_user;
			int saved_param_user_guild, saved_param_user_user;
			bool valid_stmt;
		};

		data_t data;
		database_handler& db;
	public:
		db_delete(dpp::cluster& cluster, database_handler& in_db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const override;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override;

	private:
		dpp::task<void> single(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> guild(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> self(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const;
		dpp::task<void> user(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const;

		dpp::task<void> exec(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, const size_t stmt, const uint64_t target) const;

		dpp::task<void> help(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_DELETE_H