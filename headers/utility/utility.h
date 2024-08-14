#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/message.h>

#include <optional>
#include <string>

namespace mln {
	class utility {
	public:
		//TODO fix the prints, they can cause crashes from what I have seen (i could just delete them to be honest)
		static void print(const dpp::cluster& bot, const dpp::component& com);
		static void print(const dpp::cluster& bot, const dpp::attachment& att);
		static void print(const dpp::cluster& bot, const dpp::message& msg);
		static void print(const dpp::cluster& bot, const dpp::message_file_data& msg_fd);
		static void print(const dpp::cluster& bot, const dpp::message::message_interaction_struct& msg_i);
		static dpp::task<std::optional<dpp::guild_member>> resolve_guild_member(const dpp::interaction_create_t& event);
		static dpp::task<void> send_msg_recursively(dpp::cluster& bot, const dpp::interaction_create_t& event, const std::string& msg, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast);
	};
}

#endif //H_MLN_DB_UTILITY_H