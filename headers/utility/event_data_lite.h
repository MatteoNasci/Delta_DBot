#pragma once
#ifndef H_MLN_DB_EVENT_DATA_LITE_H
#define H_MLN_DB_EVENT_DATA_LITE_H

#include <cstdint>
#include <string>

namespace dpp {
	class cluster;
	struct interaction_create_t;
}

namespace mln {
	struct event_data_lite_t {
		uint64_t command_id;
		uint64_t guild_id;
		uint64_t channel_id;
		uint64_t usr_id;
		uint64_t app_id;
		std::string command_name;
		std::string token;
		dpp::cluster* creator;
		bool is_first_reply;

		event_data_lite_t() noexcept;
		event_data_lite_t(event_data_lite_t&& rhs) noexcept;
		event_data_lite_t(const event_data_lite_t& rhs) noexcept;
		event_data_lite_t& operator=(const event_data_lite_t&) noexcept;
		event_data_lite_t& operator=(event_data_lite_t&&) noexcept;

		event_data_lite_t(const uint64_t command_id, const uint64_t guild_id, const uint64_t channel_id, const uint64_t usr_id, const uint64_t app_id, const std::string& command_name, const std::string& token, dpp::cluster* cluster, const bool is_first_reply) noexcept;
		event_data_lite_t(const dpp::interaction_create_t* const event_data, const bool is_first_reply) noexcept;
		event_data_lite_t(const dpp::interaction_create_t& event_data, const bool is_first_reply) noexcept;
		event_data_lite_t(const dpp::interaction_create_t* const event_data, dpp::cluster& cluster, const bool is_first_reply) noexcept;
		event_data_lite_t(const dpp::interaction_create_t& event_data, dpp::cluster& cluster, const bool is_first_reply) noexcept;
	};
}

#endif // H_MLN_DB_EVENT_DATA_LITE_H