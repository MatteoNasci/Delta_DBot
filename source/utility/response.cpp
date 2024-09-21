#include "utility/response.h"

dpp::async<dpp::confirmation_callback_t> mln::response::make_response(bool first_response, const dpp::interaction_create_t& event_data, dpp::message& message)
{
	if (!first_response) {
		return event_data.co_edit_response(message);
	}

	return event_data.co_reply(message.set_flags(dpp::m_ephemeral));
}

dpp::async<dpp::confirmation_callback_t> mln::response::make_response(bool first_response, const dpp::interaction_create_t& event_data, const std::string& message)
{
	if (!first_response) {
		return event_data.co_edit_response(message);
	}

	return event_data.co_reply(dpp::message{ message }.set_flags(dpp::m_ephemeral));
}
