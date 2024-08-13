#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>

#include <optional>
#include <string>

namespace mln {
	class utility {
	public:
		static dpp::task<std::optional<dpp::guild_member>> resolve_guild_member(const dpp::interaction_create_t& event);
		static dpp::task<void> send_msg_recursively(dpp::cluster& bot, const dpp::interaction_create_t& event, const std::string& msg, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast);
	};
}

#endif //H_MLN_DB_UTILITY_H