#pragma once
#ifndef H_MLN_DB_CACHES_H
#define H_MLN_DB_CACHES_H

#include "utility/cache.h"

#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/user.h>
#include <dpp/message.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/role.h>

#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include <functional>
#include <atomic>

namespace mln {
	class database_handler;
	/**
	 * @brief Class that contains all caches used by the bot. 
	 * All shared_ptr returned by the caches are not thread safe when modified, so any attempt to directly modify the contents of a shared_ptr returned should be avoided.
	 * Prefer to either get a copy of the contents and modify that, or perform an add_element on the cache to update the element you want to modify.
	 * The caches themselves are thread safe.
	 * The getter functions will try to retrieve the data from the appropriate cache, if not present they will look in dpp's resolved items (if provided), if not found they might return std::nullopt or they might attempt to contact the database (only a selected few functions).
	 * If the normal getter doesn't return a value, then use the task overload of that getter and an attempt will be made to retrieve the data from discord (and update the related cache)
	*/
	class caches {	
	private:
		static std::atomic_ullong s_cache_misses;
		static std::atomic_ullong s_cache_requests;

		static dpp::cluster* s_cluster;
		static database_handler* s_db;
	public:
		caches() = delete;

		static void init(dpp::cluster* cluster, database_handler* db);
		static void cleanup();
		static bool is_initialized();
		static unsigned long long get_total_cache_requests();
		static unsigned long long get_total_cache_misses();
		static double get_cache_misses_rate();

		struct composite_tuple_hash {
			size_t operator()(const std::tuple<uint64_t, uint64_t>& key) const;
		};

		static cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true> dump_channels_cache;
		static std::optional<uint64_t> get_dump_channel_id(uint64_t guild_id);

		static std::optional<dpp::message> get_message(uint64_t message_id, const dpp::interaction_create_t* const event_data);
		/**
		 * @brief Requires view channel and read message history perms for the bot
		*/
		static dpp::task<std::optional<dpp::message>> get_message_task(uint64_t message_id, uint64_t channel_id);

		static cache<uint64_t, std::vector<std::string>, false, 400, 30, 0.7, true> show_all_cache;
		static std::optional<std::shared_ptr<const std::vector<std::string>>> get_show_all(uint64_t guild_id);

		static cache<std::tuple<uint64_t, uint64_t>, std::vector<std::string>, false, 1000, 100, 0.7, true, composite_tuple_hash> show_user_cache;
		static std::optional<std::shared_ptr<const std::vector<std::string>>> get_show_user(const std::tuple<uint64_t, uint64_t>& guild_user_ids);

		static cache<uint64_t, dpp::guild, false, 3000, 300, 0.7, true> guild_cache;
		static std::optional<std::shared_ptr<const dpp::guild>> get_guild(uint64_t guild_id);
		static dpp::task<std::optional<std::shared_ptr<const dpp::guild>>> get_guild_task(uint64_t guild_id);
		
		static cache<uint64_t, dpp::channel, false, 4000, 300, 0.7, true> channel_cache;
		static std::optional<std::shared_ptr<const dpp::channel>> get_channel(uint64_t channel_id, const dpp::interaction_create_t* event_data);
		static dpp::task<std::optional<std::shared_ptr<const dpp::channel>>> get_channel_task(uint64_t channel_id);

		static cache<uint64_t, dpp::user_identified, false, 6000, 500, 0.7, true> user_cache;
		static std::optional<std::shared_ptr<const dpp::user_identified>> get_user(uint64_t user_id, const dpp::interaction_create_t* event_data);
		static dpp::task<std::optional<std::shared_ptr<const dpp::user_identified>>> get_user_task(uint64_t user_id);

		static cache<std::tuple<uint64_t, uint64_t>, dpp::guild_member, false, 6000, 500, 0.7, true, composite_tuple_hash> member_cache;
		static std::optional<std::shared_ptr<const dpp::guild_member>> get_member(const std::tuple<uint64_t, uint64_t>& guild_user_ids, const dpp::interaction_create_t* event_data);
		static dpp::task<std::optional<std::shared_ptr<const dpp::guild_member>>> get_member_task(const std::tuple<uint64_t, uint64_t>& guild_user_ids);

		static cache<uint64_t, dpp::role, false, 6000, 500, 0.7, true> role_cache;
		static std::optional<std::shared_ptr<const dpp::role>> get_role(uint64_t role_id, const dpp::interaction_create_t* event_data);
		static dpp::task<std::optional<std::shared_ptr<const dpp::role>>> get_role_task(uint64_t guild_id, uint64_t role_id, bool add_all_guild_roles = false);
	};
}

#endif // H_MLN_DB_CACHES_H