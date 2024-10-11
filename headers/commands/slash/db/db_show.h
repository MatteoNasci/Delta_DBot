#pragma once
#ifndef H_MLN_DB_DB_SHOW_H
#define H_MLN_DB_DB_SHOW_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/db_saved_stmt_state.h"

#include <dpp/coro/task.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	class database_handler;
	struct db_cmd_data_t;

	class db_show : public base_db_command {
	private:
		struct exec_show_t {
			size_t stmt;
			uint64_t target;
		};
		struct data_t {
			size_t saved_stmt_all, saved_stmt_user;
			int saved_param_guild, saved_param_user;
			db_saved_stmt_state valid_all, valid_user;
		};
		data_t data;
		database_handler& db;
	public:
		db_show(dpp::cluster& cluster, database_handler& db);
		~db_show();
		db_show(const db_show&) = delete;
		db_show(db_show&& rhs) noexcept;
		db_show& operator=(const db_show&) = delete;
		db_show& operator=(db_show&& rhs) noexcept;

		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const noexcept override final;
		db_saved_stmt_state is_db_initialized() const noexcept override final;
	private:
		dpp::task<void> all(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> user(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const;
		dpp::task<void> help(db_cmd_data_t& cmd_data) const;

		dpp::task<void> execute_show(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const exec_show_t& stmt_data, const std::optional<std::shared_ptr<const std::vector<std::string>>>& cached_show) const;
	};
}

#endif //H_MLN_DB_DB_SHOW_H