#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "bot_delta_data.h"
#include "general/events.h"
#include "database/database_handler.h"

#include <string>

namespace mln {
	class bot_delta {
	public:
		bot_delta_data_t data;
		database_handler db;
	public:
		~bot_delta();
		bot_delta(const bool register_cmds);
		bot_delta() = delete;
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

		std::string start();
		bool close();

		db_result insert_new_guild_id(const dpp::snowflake& guild_id);
		db_result update_guild_db_channel_id(const dpp::snowflake& guild_id, const dpp::snowflake& channel_id);
		db_result select_guild_db_channel_id(const dpp::snowflake& guild_id, bool& out_valid_channel, dpp::snowflake& out_channel_id);

		db_result print_main_db();
	private:
		size_t saved_insert_guild_query;
		size_t saved_update_guild_channel_query;
		int saved_ugc_guild_index;
		int saved_ugc_channel_index;
		size_t saved_select_guild_channel_query;
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
		database_callbacks_t get_select_callbacks(std::pair<bool, dpp::snowflake>& out_res);
		database_callbacks_t get_select_all_callbacks();
		void setup_db();
	public:
		static void initialize_environment();
		static void shutdown_environment();
	};
}

#endif //H_MLN_DB_BOT_DELTA_H