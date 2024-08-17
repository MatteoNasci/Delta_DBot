#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include "database/database_callbacks.h"
#include "commands/slash/db/url_type.h"

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/message.h>

#include <optional>
#include <string>
#include <functional>

namespace mln {
	class utility {
	public:
		//TODO fix the prints, they can cause crashes from what I have seen (i could just delete them to be honest)
		static void print(const dpp::cluster& bot, const dpp::component& com);
		static void print(const dpp::cluster& bot, const dpp::attachment& att);
		static void print(const dpp::cluster& bot, const dpp::message& msg);
		static void print(const dpp::cluster& bot, const dpp::message_file_data& msg_fd);
		static void print(const dpp::cluster& bot, const dpp::message::message_interaction_struct& msg_i);

		static std::string get_string(url_type type);

		static const database_callbacks_t& get_any_results_callback();
		
		static dpp::task<std::optional<dpp::guild_member>> resolve_guild_member(const dpp::interaction_create_t& event_data, const dpp::snowflake& user_id);
		
		static dpp::task<void> send_msg_recursively(dpp::cluster& bot, const dpp::interaction_create_t& event, const std::string& msg, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast);
		static dpp::task<bool> send_msg_recursively_embed(dpp::cluster& bot, const dpp::interaction_create_t& event, const std::function<std::string(size_t index, size_t requested_size, size_t max_size)>& get_text_callback, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast);
	
	private:
		static void any_results_da_callback(void*, int, mln::db_column_data_t&&);
		static bool any_results_td_callback(void*, int);
	};
}

#endif //H_MLN_DB_UTILITY_H