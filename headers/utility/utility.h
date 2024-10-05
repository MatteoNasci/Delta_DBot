#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include "database/database_callbacks.h"
#include "database/db_column_data.h"
#include "utility/event_data_lite.h"

#include <dpp/appcommand.h>
#include <dpp/coro/job.h>
#include <dpp/coro/task.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace dpp {
	class cluster;
	struct message;
	class guild;
}

namespace mln {
	class utility {
	public:
		struct paginated_data_t {
			event_data_lite_t event_data;
			uint64_t time_limit_seconds, text_limit;
		};

		[[nodiscard]] static const database_callbacks_t& get_any_results_callback();
		[[nodiscard]] static database_callbacks_t get_any_results_callback(bool* found);
		
		static dpp::job manage_paginated_embed(const paginated_data_t data, const std::shared_ptr<const std::vector<std::string>> text_ptr);

		[[nodiscard]] static dpp::task<std::optional<std::string>> check_text_validity(const dpp::command_value& text, event_data_lite_t& lite_data, const bool can_be_empty, const size_t min_size, const size_t max_size, const std::string& err_text);
		[[nodiscard]] static dpp::task<bool> check_text_validity(const std::string& text, event_data_lite_t& lite_data, const bool can_be_empty, const size_t min_size, const size_t max_size, const std::string& err_text);

		[[nodiscard]] static unsigned int to_digit(const char character);
		[[nodiscard]] static bool is_digit(const char character);
		[[nodiscard]] static bool is_ascii(const std::string& text);
		[[nodiscard]] static bool is_ascii(const char* const text);
		[[nodiscard]] static bool is_ascii(const unsigned char* const text);

		[[nodiscard]] static bool is_ascii_printable(const std::string& text);
		[[nodiscard]] static bool is_ascii_printable(const char* const text);
		[[nodiscard]] static bool is_ascii_printable(const unsigned char* const text);

		[[nodiscard]] static std::vector<dpp::snowflake> extract_emojis(const std::string& content);

		static bool conf_callback_is_error(const dpp::confirmation_callback_t& callback, const event_data_lite_t& event_data, const bool always_event_log, const std::string& additional_msg);
		[[nodiscard]] static std::function<void(const dpp::confirmation_callback_t&)> get_conf_callback_is_error(const event_data_lite_t& event_data, const bool always_event_log, const std::string& additional_msg);
		static void create_event_log_error(const event_data_lite_t& event_data, const std::string& additional_msg);
		
		[[nodiscard]] static bool is_same_cmd(const dpp::slashcommand& first, const dpp::slashcommand& second);
		[[nodiscard]] static bool is_same_option(const dpp::command_option& first, const dpp::command_option& second);
		[[nodiscard]] static bool is_same_choice(const dpp::command_option_choice& first, const dpp::command_option_choice& second);

		[[nodiscard]] static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		[[nodiscard]] static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		[[nodiscard]] static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		[[nodiscard]] static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		[[nodiscard]] static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		[[nodiscard]] static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		[[nodiscard]] static bool extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id);

		[[nodiscard]] static constexpr bool is_dev_build();
		[[nodiscard]] static std::string prefix_dev(const char* const text);
	private:
		static void any_results_da_callback(void*, int, mln::db_column_data_t&&);
		static bool any_results_td_callback(void*, int);
	};
}

#endif //H_MLN_DB_UTILITY_H