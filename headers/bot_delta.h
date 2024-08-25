#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "events/cmd_ctx_runner.h"
#include "events/cmd_runner.h"
#include "events/guild_create_runner.h"
#include "events/ready_runner.h"
#include "database/database_handler.h"
#include "utility/cache.h"

#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/user.h>
#include <dpp/cluster.h>

#include <string>
#include <optional>
#include <memory>

namespace mln {
	struct composite_tuple_hash {
		std::size_t operator()(const std::tuple<uint64_t, uint64_t>& key) const {

			std::size_t h1 = std::hash<uint64_t>{}(std::get<0>(key));
			std::size_t h2 = std::hash<uint64_t>{}(std::get<0>(key));

			// Combine the hash values
			return h1 ^ (h2 << 1);
		}
	};

	class bot_delta {
	public:
		dpp::cluster bot;
		const dpp::snowflake dev_id;
		const bool is_dev_id_valid;
		bool registered_new_cmds;

		database_handler db;
		cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true> dump_channels_cache;
		cache<uint64_t, dpp::message, true, 5000, 500, 0.7, true> messages_cache;
		cache<uint64_t, std::vector<std::string>, false, 400, 30, 0.7, true> show_all_cache;
		cache<std::tuple<uint64_t, uint64_t>, std::vector<std::string>, false, 1000, 100, 0.7, true, composite_tuple_hash> show_user_cache;
		cache<uint64_t, dpp::guild, false, 400, 30, 0.7, true> guild_cache;
		cache<uint64_t, dpp::channel, false, 2500, 300, 0.7, true> channel_cache;
		cache<uint64_t, dpp::user, false, 6000, 500, 0.7, true> user_cache;
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

		std::string start(bool register_cmds);
		bool close();

		db_result print_main_db() const;
		db_result recalculate_dump_channels_cache();
		std::optional<uint64_t> get_db_dump_channel(uint64_t guild_id);


		const cmd_runner& get_cmd_runner() const;
		const cmd_ctx_runner& get_cmd_ctx_runner() const;
	private:
		size_t saved_select_all_query;
		size_t saved_select_all_gp_query;
		size_t saved_select_dump_channel;

		cmd_runner cmds;
		cmd_ctx_runner ctxs;
		ready_runner readys;
		guild_create_runner guild_creates;
	private:
		void init();
		void setup_db();

	public:
		struct upd_dump_channel_t {
			cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true>& cache;
			uint64_t guild;
		};

		static void initialize_environment();
		static void shutdown_environment();

		static size_t max_text_id_size();
		static size_t min_text_id_size();
	};
}

#endif //H_MLN_DB_BOT_DELTA_H