#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "general/events.h"
#include "database/database_handler.h"

#include <dpp/cluster.h>

#include <string>

namespace mln {
	class bot_delta {
	public:
		dpp::cluster bot;
		const dpp::snowflake dev_id;
		const bool is_dev_id_valid;
		bool registered_new_cmds;

		database_handler db;
	public:
		~bot_delta();
		/**
		 * @brief bot_delta is non-copyable
		 */
		bot_delta(const bot_delta&) = delete;

		/**
		 * @brief bot_delta is non-moveable
		 */
		bot_delta(const bot_delta&&) = delete;

		/**
		 * @brief bot_delta is non-copyable
		 */
		bot_delta& operator=(const bot_delta&) = delete;

		/**
		 * @brief bot_delta is non-moveable
		 */
		bot_delta& operator=(const bot_delta&&) = delete;

		std::string start(bool register_cmds);
		bool close();

		db_result print_main_db() const;
	private:
		size_t saved_select_all_query;

		cmd_runner cmds;
		cmd_ctx_runner ctxs;
		form_submit_runner forms;
		select_click_runner selects;
		ready_runner readys;
		msg_react_remove_runner react_removes;
		message_create_runner msg_creates;
		button_click_runner button_clicks;
		autocomplete_runner autocompletes;
	private:
		void init();
		void setup_db();

		bot_delta();
	public:
		static void initialize_environment();
		static void shutdown_environment();

		static bot_delta& delta() {
			static bot_delta delta;
			return delta;
		}
	};
}

#endif //H_MLN_DB_BOT_DELTA_H