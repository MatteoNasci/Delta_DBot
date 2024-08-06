#pragma once
#ifndef H_MLN_DB_BOT_DELTA_DATA_H
#define H_MLN_DB_BOT_DELTA_DATA_H

#include <dpp/cluster.h>
#include <dpp/snowflake.h>

namespace mln {
	struct bot_delta_data_t {
		dpp::cluster bot;
		const dpp::snowflake dev_id;
		const bool is_dev_id_valid;
		const bool registered_new_cmds;
		bot_delta_data_t(dpp::snowflake in_dev_id, bool in_is_dev_id_valid, bool in_registered_new_cmds);
		bot_delta_data_t() = delete;
		/**
		 * @brief bot_delta_data_t is non-copyable
		 */
		bot_delta_data_t(const bot_delta_data_t&) = delete;
		/**
		 * @brief bot_delta_data_t is non-moveable
		 */
		bot_delta_data_t(const bot_delta_data_t&&) = delete;
		/**
		 * @brief bot_delta_data_t is non-copyable
		 */
		bot_delta_data_t& operator=(const bot_delta_data_t&) = delete;
		/**
		 * @brief bot_delta_data_t is non-moveable
		 */
		bot_delta_data_t& operator=(const bot_delta_data_t&&) = delete;
	};
}

#endif //H_MLN_DB_BOT_DELTA_DATA_H