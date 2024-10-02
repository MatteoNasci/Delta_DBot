#pragma once
#ifndef H_MLN_DB_DB_DELETE_H
#define H_MLN_DB_DB_DELETE_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"

#include <dpp/coro/task.h>

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>


namespace dpp {
	class cluster;
	struct slashcommand_t;
	struct interaction_create_t;
}

namespace mln {
	class database_handler;
	struct db_cmd_data_t;
	struct event_data_lite_t;

	class db_delete : public base_db_command {
	private:

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
		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) const override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override final;
		bool is_db_initialized() const override final;
	private:
		std::string get_warning_message(const db_command_type type) const;

		dpp::task<void> single(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> guild(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> self(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> user(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const;

		dpp::task<void> exec(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data, const size_t stmt, const uint64_t target) const;

		dpp::task<void> help(db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_DELETE_H