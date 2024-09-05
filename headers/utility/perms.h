#pragma once
#ifndef H_MLN_DB_PERMS_H
#define H_MLN_DB_PERMS_H

#include <dpp/permissions.h>
#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/task.h>

#include <optional>
#include <vector>

namespace mln {
	class perms {
	public:
		static std::optional<dpp::permission> get_base_permission(const dpp::guild& guild, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data);
		static dpp::task<std::optional<dpp::permission>> get_base_permission_task(const dpp::guild& guild, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data);
		
		static std::optional<dpp::permission> get_overwrite_permission(const dpp::permission& base_permission, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data);

		static std::optional<dpp::permission> get_computed_permission(const dpp::guild& guild, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data);
		static dpp::task<std::optional<dpp::permission>> get_computed_permission_task(const dpp::guild& guild, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data);
	
		static dpp::task<dpp::permission> get_additional_perms_required(const dpp::message& msg, dpp::cluster& bot, uint64_t guild_id);

		static bool check_permissions(const std::vector<dpp::permission>& permissions, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const std::vector<dpp::permission>& permissions, dpp::permissions permission_to_check);
		static bool check_permissions(const std::vector<dpp::permission>& permissions, uint64_t permission_to_check);
		static bool check_permissions(dpp::permission permission, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(dpp::permission permission, dpp::permissions permission_to_check);
		static bool check_permissions(dpp::permission permission, uint64_t permission_to_check);
	};
}

#endif //H_MLN_DB_PERMS_H