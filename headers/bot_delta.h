#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "events/cmd_ctx_runner.h"
#include "events/cmd_runner.h"
#include "events/guild_create_runner.h"
#include "events/ready_runner.h"
#include "database/database_handler.h"

#include <dpp/cluster.h>
#include <dpp/timer.h>

#include <string>
#include <optional>
#include <memory>
#include <atomic>

namespace mln {

	class bot_delta {
	public:
		dpp::cluster bot;
		const dpp::snowflake dev_id;
		const bool is_dev_id_valid;

		database_handler db;

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

		const cmd_runner& get_cmd_runner() const;
		const cmd_ctx_runner& get_cmd_ctx_runner() const;
		const bool is_bot_running() const;
	private:
		dpp::timer db_optimize_timer;

		size_t saved_optimize_db;
		size_t saved_select_all_query;
		size_t saved_select_all_gp_query;

		cmd_runner cmds;
		cmd_ctx_runner ctxs;
		ready_runner readys;
		guild_create_runner guild_creates;

		std::atomic_bool running;

		struct file_closer_t {
			void operator()(FILE* f) const;
		};
		std::unique_ptr<FILE, file_closer_t> log_file;
	private:
		void init();
		void setup_db();

		static void logger(const dpp::log_t& log);
		static void log_to_file(const dpp::log_t& log);

	public:
		static void initialize_environment();
		static void shutdown_environment();

		static size_t max_text_id_size();
		static size_t min_text_id_size();
	};
}

#endif //H_MLN_DB_BOT_DELTA_H