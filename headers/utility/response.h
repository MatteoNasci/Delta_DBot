#pragma once
#ifndef H_MLN_DB_RESPONSE_H
#define H_MLN_DB_RESPONSE_H

#include <dpp/coro/async.h>
#include <dpp/coro/task.h>

#include <functional>
#include <string>

namespace dpp {
	class cluster;
	struct message;
	struct interaction_create_t;
	struct confirmation_callback_t;
}

namespace mln {
	struct event_data_lite_t;

	class response {
	public:
		static dpp::async<dpp::confirmation_callback_t> co_make_response(const bool is_first_response, const dpp::interaction_create_t& event_data, const dpp::message& message);
		static dpp::async<dpp::confirmation_callback_t> co_make_response(const bool is_first_response, const dpp::interaction_create_t& event_data, const std::string& message);
		static dpp::async<dpp::confirmation_callback_t> co_make_response(event_data_lite_t& event_data, const dpp::message& message);
		static dpp::async<dpp::confirmation_callback_t> co_make_response(event_data_lite_t& event_data, const std::string& message);
		static void make_response(const bool is_first_response, const dpp::interaction_create_t& event_data, const dpp::message& message, std::function<void(const dpp::confirmation_callback_t&)> callback);
		static void make_response(const bool is_first_response, const dpp::interaction_create_t& event_data, const std::string& message, std::function<void(const dpp::confirmation_callback_t&)> callback);
		static void make_response(event_data_lite_t& event_data, const dpp::message& message, std::function<void(const dpp::confirmation_callback_t&)> callback);
		static void make_response(event_data_lite_t& event_data, const std::string& message, std::function<void(const dpp::confirmation_callback_t&)> callback);

		static void respond(event_data_lite_t& lite_data, std::string respond_msg, const bool always_log, std::string log_msg);
		static void respond(event_data_lite_t& lite_data, const dpp::message& respond_msg, const bool always_log, std::string log_msg);
		static dpp::task<bool> co_respond(event_data_lite_t& lite_data, std::string respond_msg, const bool always_log, std::string log_msg);
		static dpp::task<bool> co_respond(event_data_lite_t& lite_data, const dpp::message& respond_msg, const bool always_log, std::string log_msg);

		static void think(event_data_lite_t& lite_data, const bool ephemeral, const bool always_log, std::string log_msg);
		static dpp::task<bool> co_think(event_data_lite_t& lite_data, const bool ephemeral, const bool always_log, std::string log_msg);
	
		static [[nodiscard]] bool is_event_data_valid(const event_data_lite_t& event_data);
	private:
		static void check_event_data_validity(const event_data_lite_t& event_data);
	};
}

#endif //H_MLN_DB_RESPONSE_H