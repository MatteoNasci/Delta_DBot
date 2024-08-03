#pragma once
#ifndef H_MLN_DB_BOT_DELTA_DATA_H
#define H_MLN_DB_BOT_DELTA_DATA_H

#include <dpp/cluster.h>
#include <dpp/snowflake.h>

struct bot_delta_data_t{
    dpp::cluster bot; 
    const dpp::snowflake dev_id;
    const bool is_dev_id_valid; 
    bot_delta_data_t(dpp::snowflake in_dev_id, bool in_is_dev_id_valid);
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
typedef bot_delta_data_t bd_data_t;

#endif