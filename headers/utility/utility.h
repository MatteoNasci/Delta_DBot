#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include "database/database_callbacks.h"

#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/coro/async.h>
#include <dpp/restresults.h>
#include <dpp/permissions.h>
#include <dpp/appcommand.h>

#include <optional>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace dpp {
	class cluster;
	class message;
	class guild;
}

namespace mln {
	class utility {
	public:
		struct paginated_data_t {
			std::string token;
			dpp::cluster& bot;
			uint64_t guild_id, channel_id, event_id;
			uint64_t time_limit_seconds, text_limit;
		};

		static const database_callbacks_t& get_any_results_callback();
		static database_callbacks_t get_any_results_callback(bool* found);
		
		static dpp::job manage_paginated_embed(paginated_data_t data, const std::shared_ptr<const std::vector<std::string>> text_ptr);

		static bool is_ascii(const std::string& text);
		static bool is_ascii(const char* const text);
		static bool is_ascii(const unsigned char* const text);

		static bool is_ascii_printable(const std::string& text);
		static bool is_ascii_printable(const char* const text);
		static bool is_ascii_printable(const unsigned char* const text);

		static std::vector<dpp::snowflake> extract_emojis(const std::string& content);

		static bool conf_callback_is_error(const dpp::confirmation_callback_t& callback, const dpp::cluster& bot, const dpp::interaction_create_t* const event_data = nullptr, const std::string& additional_msg = {});
		static void create_event_log_error(const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& additional_msg = {});
		
		static bool is_same_cmd(const dpp::slashcommand& first, const dpp::slashcommand& second);
		static bool is_same_option(const dpp::command_option& first, const dpp::command_option& second);
		static bool is_same_choice(const dpp::command_option_choice& first, const dpp::command_option_choice& second);

		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id);

		static std::string get_current_date_time();
	private:
		static void any_results_da_callback(void*, int, mln::db_column_data_t&&);
		static bool any_results_td_callback(void*, int);
	};
}

#endif //H_MLN_DB_UTILITY_H