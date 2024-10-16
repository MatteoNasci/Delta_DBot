#pragma once
#ifndef H_MLN_DB_DB_INSERT_H
#define H_MLN_DB_DB_INSERT_H

#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/db_saved_stmt_state.h"
#include "utility/url.h"

#include <dpp/coro/task.h>

#include <string>
#include <utility>

namespace dpp {
	class cluster;
	struct slashcommand_t;
}

namespace mln {
	class database_handler;
	struct db_cmd_data_t;
	struct event_data_lite_t;

	class db_insert : public base_db_command {
	private:

		struct data_t {
			size_t saved_stmt;
			int saved_param_guild, saved_param_name, saved_param_url, saved_param_user, saved_param_desc, saved_param_nsfw;
			db_saved_stmt_state state;
		};

		data_t data;
		database_handler& db;
	public:
		
		db_insert(dpp::cluster& cluster, database_handler& db);
		~db_insert();
		db_insert(const db_insert&) = delete;
		db_insert(db_insert&& rhs) noexcept;
		db_insert& operator=(const db_insert&) = delete;
		db_insert& operator=(db_insert&& rhs) noexcept;

		dpp::task<void> command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) override final;
		db_init_type_flag get_requested_initialization_type(const db_command_type cmd) const noexcept override final;
		db_saved_stmt_state is_db_initialized() const noexcept override final;
	private:

		dpp::task<void> command_url(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, bool nsfw);
		dpp::task<void> command_file(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, bool nsfw);
		dpp::task<void> command_text(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, bool nsfw);
		dpp::task<void> command_help(db_cmd_data_t& cmd_data) const;

		dpp::task<bool> execute_query(const dpp::slashcommand_t& event_data, event_data_lite_t& current_event, db_cmd_data_t& cmd_data, const std::string& url, bool nsfw) const;
		dpp::task<void> manage_attach_url(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const std::tuple<std::string, std::string>& url_name, bool nsfw);
		dpp::task<void> manage_msg_url(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const msg_url_t& url_ids, bool nsfw);
	};
}

#endif //H_MLN_DB_DB_INSERT_H