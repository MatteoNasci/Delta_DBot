#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "database/database_handler.h"
#include "database/db_result.h"
#include "events/cmd_ctx_runner.h"
#include "events/cmd_runner.h"
#include "events/guild_create_runner.h"
#include "events/ready_runner.h"
#include "threads/jobs_runner.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <dpp/timer.h>

#include <atomic>
#include <cstdint>
#include <string>

namespace mln {

	class bot_delta {
	private:
		std::atomic_bool running;
		dpp::timer db_optimize_timer;

		size_t saved_optimize_db;
		size_t saved_select_all_query;
		size_t saved_select_all_gp_query;

		const dpp::snowflake dev_id;
		const bool is_dev_id_valid;

		database_handler database;
		dpp::cluster bot;

		jobs_runner j_runner;

		cmd_runner cmds;
		cmd_ctx_runner ctxs;
		ready_runner readys;
		guild_create_runner guild_creates;

	public:
		bot_delta();

		~bot_delta();
		/**
		 * @brief bot_delta is non-copyable
		 */
		bot_delta(const bot_delta&) = delete;

		/**
		 * @brief bot_delta is non-moveable
		 */
		bot_delta(bot_delta&&) = delete;

		/**
		 * @brief bot_delta is non-copyable
		 */
		bot_delta& operator=(const bot_delta&) = delete;

		/**
		 * @brief bot_delta is non-moveable
		 */
		bot_delta& operator=(bot_delta&&) = delete;

		std::string start();
		bool close();

		db_result_t print_main_db() const;
		void print_thread_data() const;

		const cmd_runner& get_cmd_runner() const;
		const cmd_ctx_runner& get_cmd_ctx_runner() const;
		const bool is_bot_running() const;

		void log(const dpp::loglevel severity, const std::string& msg) const;
		void set_request_timeout(const uint16_t timeout = 20);
	private:
		void init();
		void setup_db();

		static void logger(const dpp::log_t& log);

	public:
		static void initialize_environment();
		static void shutdown_environment();

		[[nodiscard]] inline static constexpr size_t max_text_id_size() noexcept {
			return 50;
		}
		[[nodiscard]] inline static constexpr size_t min_text_id_size() noexcept {
			return 1;
		}
	};
}

#endif //H_MLN_DB_BOT_DELTA_H