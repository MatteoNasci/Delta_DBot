#pragma once
#ifndef H_MLN_DB_PERMS_H
#define H_MLN_DB_PERMS_H

#include <dpp/coro/task.h>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace dpp {
	class guild_member;
	class channel;
	class role;
	struct message;
}

namespace mln {
	struct event_data_lite_t;

	class perms {
	public:
		static std::optional<dpp::permission> get_base_permission(const uint64_t guild_owner, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map);
		static dpp::task<std::optional<dpp::permission>> get_base_permission_task(const uint64_t guild_owner, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map);
		
		static dpp::permission get_overwrite_permission(const dpp::permission& base_permission, const dpp::channel& channel, const dpp::guild_member& member, const std::map<dpp::snowflake, dpp::permission>* const resolved_map);

		static std::optional<dpp::permission> get_computed_permission(const uint64_t guild_owner, const dpp::channel& channel, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_role_map, const std::map<dpp::snowflake, dpp::permission>* const resolved_perms_map);
		static dpp::task<std::optional<dpp::permission>> get_computed_permission_task(const uint64_t guild_owner, const dpp::channel& channel, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_role_map, const std::map<dpp::snowflake, dpp::permission>* const resolved_perms_map);

		static dpp::task<std::optional<dpp::permission>> get_additional_perms_required_task(const dpp::message& msg, const uint64_t guild_id, event_data_lite_t& lite_data);
		static std::optional<dpp::permission> get_additional_perms_required(const dpp::message& msg, const uint64_t guild_id, event_data_lite_t& lite_data);

		static bool check_permissions(const std::vector<dpp::permission>& permissions, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const std::vector<dpp::permission>& permissions, const dpp::permissions permission_to_check);
		static bool check_permissions(const std::vector<dpp::permission>& permissions, const uint64_t permission_to_check);
		static bool check_permissions(const dpp::permission permission, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::permission permission, const dpp::permissions permission_to_check);
		static bool check_permissions(const dpp::permission permission, const uint64_t permission_to_check);
	};
}

#endif //H_MLN_DB_PERMS_H