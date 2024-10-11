#pragma once
#ifndef H_MLN_DB_DB_SELECT_H
#define H_MLN_DB_DB_SELECT_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	class database_handler;
	struct db_cmd_data_t;

	class db_select : public base_db_command {

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name;
			db_saved_stmt_state state;
		};
		data_t data;
		database_handler& db;
	public:
		db_select(dpp::cluster& cluster, database_handler& db);
		~db_select();
		db_select(const db_select&) = delete;
		db_select(db_select&& rhs) noexcept;
		db_select& operator=(const db_select&) = delete;
		db_select& operator=(db_select&& rhs) noexcept;

		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const noexcept override final;
		db_saved_stmt_state is_db_initialized() const noexcept override final;
	private:
		dpp::task<void> select(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data);
		dpp::task<void> help(db_cmd_data_t& cmd_data) const;
	};
}

#endif //H_MLN_DB_DB_SELECT_H