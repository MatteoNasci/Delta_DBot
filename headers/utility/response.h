#pragma once
#ifndef H_MLN_DB_RESPONSE_H
#define H_MLN_DB_RESPONSE_H

#include <dpp/restresults.h>
#include <dpp/coro/async.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>

namespace mln {
	class response {
	public:
		static dpp::async<dpp::confirmation_callback_t> make_response(bool is_first_response, const dpp::interaction_create_t& event_data, dpp::message& message);
		static dpp::async<dpp::confirmation_callback_t> make_response(bool is_first_response, const dpp::interaction_create_t& event_data, const std::string& message);
	};
}

#endif //H_MLN_DB_RESPONSE_H