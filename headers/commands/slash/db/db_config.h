#pragma once
#ifndef H_MLN_DB_DB_CONFIG_H
#define H_MLN_DB_DB_CONFIG_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
	struct message;
}

namespace mln {
	class database_handler;
	struct db_cmd_data_t;

	class db_config final : public base_db_command {
	private:

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_channel;
			bool valid_stmt;
		};
		const dpp::message& info;
		data_t data;
		database_handler& db;
	public:
		db_config(dpp::cluster& cluster, database_handler& db);
		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) const override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const override final;
		bool is_db_initialized() const override final;
	private:
		dpp::task<void> update_dump(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> help(db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_CONFIG_H