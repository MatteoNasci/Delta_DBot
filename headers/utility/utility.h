#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include "database/database_callbacks.h"

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/message.h>
#include <dpp/coro/async.h>
#include <dpp/restresults.h>
#include <dpp/permissions.h>

#include <optional>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace mln {
	class utility {
	public:
		struct paginated_data_t {
			std::string token;
			dpp::cluster* bot;
			uint64_t guild_id, channel_id, event_id;
			uint64_t time_limit_seconds, text_limit;
		};

		static const database_callbacks_t& get_any_results_callback();
		static database_callbacks_t get_any_results_callback(bool* found);

		//checks permissions in the given channel of given guild
		
		static dpp::task<void> co_conclude_thinking_response(dpp::async<dpp::confirmation_callback_t>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always = {true, dpp::loglevel::ll_error});
		static dpp::task<void> co_conclude_thinking_response(std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always = {true, dpp::loglevel::ll_error}, bool default_reply_behaviour = true);

		static dpp::job manage_paginated_embed(paginated_data_t data, const std::shared_ptr<const std::vector<std::string>> text_ptr);

		static bool is_ascii(const std::string& text);
		static bool is_ascii(const char* const text);
		static bool is_ascii(const unsigned char* const text);

		static bool is_ascii_printable(const std::string& text);
		static bool is_ascii_printable(const char* const text);
		static bool is_ascii_printable(const unsigned char* const text);

		static std::vector<dpp::snowflake> extract_emojis(const std::string& content);

		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id);
	private:
		static void any_results_da_callback(void*, int, mln::db_column_data_t&&);
		static bool any_results_td_callback(void*, int);
	};
}

#endif //H_MLN_DB_UTILITY_H