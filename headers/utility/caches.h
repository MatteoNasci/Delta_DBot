#pragma once
#ifndef H_MLN_DB_CACHES_H
#define H_MLN_DB_CACHES_H

#include "utility/cache.h"

#include <dpp/coro/task.h>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>

#include <atomic>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace dpp {
	class cluster;
	class role;
	class user;
	class guild_member;
	class user_identified;
	class guild;
	class channel;
	struct message;
}

namespace mln {
	struct event_data_lite_t;
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

		static size_t s_saved_select_dump_channel;
		static size_t s_saved_on_guild_member_update;
		static size_t s_saved_on_channel_update;
		static size_t s_saved_on_user_update;
		static size_t s_saved_on_guild_update;
		static size_t s_saved_on_guild_role_update;

		static dpp::cluster* s_cluster;
		static database_handler* s_db;
	public:

		caches() = delete;

		static void init(dpp::cluster* cluster, database_handler* db);
		static void cleanup();
		static bool is_initialized();
		static unsigned long long get_total_cache_requests();
		static unsigned long long get_total_cache_misses();
		static long double get_cache_misses_rate();
	
		struct composite_tuple_hash {
			size_t operator()(const std::tuple<uint64_t, uint64_t>& value) const noexcept {
				const size_t hash1 = std::hash<uint64_t>()(std::get<0>(value));
				const size_t hash2 = std::hash<uint64_t>()(std::get<1>(value));

				return hash1 ^ (hash2 << 1);
			}

			composite_tuple_hash() noexcept {}
			composite_tuple_hash(const composite_tuple_hash&) noexcept {}
			composite_tuple_hash(composite_tuple_hash&&) noexcept {}
			composite_tuple_hash& operator=(const composite_tuple_hash&) noexcept { return *this; }
			composite_tuple_hash& operator=(composite_tuple_hash&&) noexcept { return *this; }
		};
		struct composite_tuple_eq {
			bool operator()(const std::tuple<uint64_t, uint64_t>& lhs, const std::tuple<uint64_t, uint64_t>& rhs) const noexcept {
				return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs);
			}

			composite_tuple_eq() noexcept {}
			composite_tuple_eq(const composite_tuple_eq&) noexcept {}
			composite_tuple_eq(composite_tuple_eq&&) noexcept {}
			composite_tuple_eq& operator=(const composite_tuple_eq&) noexcept { return *this; }
			composite_tuple_eq& operator=(composite_tuple_eq&&) noexcept { return *this; }
		};

		static cache_primitive<uint64_t, uint64_t, 10000, 1000, 0.75, true> dump_channels_cache;
		static std::optional<uint64_t> get_dump_channel_id(const uint64_t guild_id);

		/**
		 * @brief Requires view channel and read message history perms for the bot
		*/
		static dpp::task<std::optional<dpp::message>> get_message_task(const uint64_t message_id, const uint64_t channel_id, const dpp::permission bot_permissions, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::message>* const resolved_map);
		static std::optional<dpp::message> get_message(const uint64_t message_id, const uint64_t channel_id, const dpp::permission bot_permissions, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::message>* const resolved_map);

		static cache<uint64_t, std::vector<std::string>, false, 400, 30, 0.7, true> show_all_cache;
		static std::optional<std::shared_ptr<const std::vector<std::string>>> get_show_all(const uint64_t guild_id);

		static cache<std::tuple<uint64_t, uint64_t>, std::vector<std::string>, false, 1000, 100, 0.7, true, composite_tuple_hash, composite_tuple_eq> show_user_cache;
		static std::optional<std::shared_ptr<const std::vector<std::string>>> get_show_user(const uint64_t guild_id, const uint64_t user_id);

		static cache<uint64_t, dpp::guild, false, 3000, 300, 0.7, true> guild_cache;
		static dpp::task<std::optional<std::shared_ptr<const dpp::guild>>> get_guild_task(const uint64_t guild_id, event_data_lite_t& lite_data);
		static std::optional<std::shared_ptr<const dpp::guild>> get_guild(const uint64_t guild_id, event_data_lite_t& lite_data);
		
		static cache<uint64_t, dpp::channel, false, 4000, 300, 0.7, true> channel_cache;
		static dpp::task<std::optional<std::shared_ptr<const dpp::channel>>> get_channel_task(const uint64_t channel_id, event_data_lite_t& lite_data, const dpp::channel* const event_channel, const std::map<dpp::snowflake, dpp::channel>* const resolved_map);
		static std::optional<std::shared_ptr<const dpp::channel>> get_channel(const uint64_t channel_id, event_data_lite_t& lite_data, const dpp::channel* const event_channel, const std::map<dpp::snowflake, dpp::channel>* const resolved_map);
		

		static cache<uint64_t, dpp::user_identified, false, 6000, 500, 0.7, true> user_cache;
		/**
		*If a given user_identified is not filled properly, you can force an API call to retrieve the data by deleting the cache entry and calling get_user with invoking_user and resolved_map as nullptr.
		*/
		static dpp::task<std::optional<std::shared_ptr<const dpp::user_identified>>> get_user_task(const uint64_t user_id, event_data_lite_t& lite_data, const dpp::user* const invoking_usr, const std::map<dpp::snowflake, dpp::user>* const resolved_map);
		static std::optional<std::shared_ptr<const dpp::user_identified>> get_user(const uint64_t user_id, event_data_lite_t& lite_data, const dpp::user* const invoking_usr, const std::map<dpp::snowflake, dpp::user>* const resolved_map);

		static cache<std::tuple<uint64_t, uint64_t>, dpp::guild_member, false, 6000, 500, 0.7, true, composite_tuple_hash, composite_tuple_eq> member_cache;
		static dpp::task<std::optional<std::shared_ptr<const dpp::guild_member>>> get_member_task(const uint64_t guild_id, const uint64_t user_id, event_data_lite_t& lite_data, const dpp::guild_member* const invoking_usr, const std::map<dpp::snowflake, dpp::guild_member>* const resolved_map);
		static std::optional<std::shared_ptr<const dpp::guild_member>> get_member(const uint64_t guild_id, const uint64_t user_id, event_data_lite_t& lite_data, const dpp::guild_member* const invoking_usr, const std::map<dpp::snowflake, dpp::guild_member>* const resolved_map);

		static cache<uint64_t, dpp::role, false, 6000, 500, 0.7, true> role_cache;
		static dpp::task<std::optional<std::shared_ptr<const dpp::role>>> get_role_task(const uint64_t guild_id, const uint64_t role_id, const bool add_all_guild_roles, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map);
		static std::optional<std::shared_ptr<const dpp::role>> get_role(const uint64_t guild_id, const uint64_t role_id, const bool add_all_guild_roles, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map);
	};
}

#endif // H_MLN_DB_CACHES_H