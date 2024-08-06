#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "bot_delta_data.h"
#include "general/events.h"

#include <string>

namespace mln {
	class bot_delta {
	public:
		bot_delta_data_t data;
	public:
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
	private:

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
	};
}

#endif //H_MLN_DB_BOT_DELTA_H