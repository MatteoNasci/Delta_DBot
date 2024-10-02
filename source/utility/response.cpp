#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/async.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/restresults.h>

#include <exception>
#include <functional>
#include <string>
#include <type_traits>

dpp::async<dpp::confirmation_callback_t> mln::response::co_make_response(const bool first_response, const dpp::interaction_create_t& event_data, const dpp::message& message)
{
	event_data_lite_t lite_data{ event_data, first_response };
	return mln::response::co_make_response(lite_data, message);
}

dpp::async<dpp::confirmation_callback_t> mln::response::co_make_response(const bool first_response, const dpp::interaction_create_t& event_data, const std::string& message)
{
	event_data_lite_t lite_data{ event_data, first_response };
	return mln::response::co_make_response(lite_data, message);
}

dpp::async<dpp::confirmation_callback_t> mln::response::co_make_response(event_data_lite_t& event_data, const dpp::message& message)
{
	mln::response::check_event_data_validity(event_data);

	const bool use_reply = event_data.is_first_reply;
	event_data.is_first_reply = false;

	if (!use_reply) [[likely]] {
		return event_data.creator->co_interaction_response_edit(event_data.token, message);
	}

	return event_data.creator->co_interaction_response_create(event_data.command_id, event_data.token, dpp::interaction_response{ dpp::ir_channel_message_with_source, message });
}

dpp::async<dpp::confirmation_callback_t> mln::response::co_make_response(event_data_lite_t& event_data, const std::string& message)
{
	mln::response::check_event_data_validity(event_data);

	const bool use_reply = event_data.is_first_reply;
	event_data.is_first_reply = false;

	if (!use_reply) [[likely]] {
		return event_data.creator->co_interaction_response_edit(event_data.token, dpp::message{ message });
	}

	return event_data.creator->co_interaction_response_create(event_data.command_id, event_data.token, dpp::interaction_response{ dpp::ir_channel_message_with_source, dpp::message{ message }.set_flags(dpp::m_ephemeral) });
}

void mln::response::make_response(const bool first_response, const dpp::interaction_create_t& event_data, const dpp::message& message, std::function<void(const dpp::confirmation_callback_t&)> callback)
{
	event_data_lite_t lite_data{ event_data, first_response };
	mln::response::make_response(lite_data, message, std::move(callback));
}

void mln::response::make_response(const bool first_response, const dpp::interaction_create_t& event_data, const std::string& message, std::function<void(const dpp::confirmation_callback_t&)> callback)
{
	event_data_lite_t lite_data{ event_data, first_response };
	mln::response::make_response(lite_data, message, std::move(callback));
}

void mln::response::make_response(event_data_lite_t& event_data, const dpp::message& message, std::function<void(const dpp::confirmation_callback_t&)> callback)
{
	mln::response::check_event_data_validity(event_data);

	const bool use_reply = event_data.is_first_reply;
	event_data.is_first_reply = false;

	if (!use_reply) [[likely]] {
		event_data.creator->interaction_response_edit(event_data.token, message, std::move(callback));
		return;
	}

	event_data.creator->interaction_response_create(
		event_data.command_id,
		event_data.token,
		dpp::interaction_response{ dpp::ir_channel_message_with_source, message },
		std::move(callback)
	);
}

void mln::response::make_response(event_data_lite_t& event_data, const std::string& msg, std::function<void(const dpp::confirmation_callback_t&)> callback)
{
	mln::response::check_event_data_validity(event_data);

	const bool use_reply = event_data.is_first_reply;
	event_data.is_first_reply = false;

	if (!use_reply) [[likely]] {
		event_data.creator->interaction_response_edit(event_data.token, dpp::message{ msg }, std::move(callback));
		return;
	}

	event_data.creator->interaction_response_create(
		event_data.command_id,
		event_data.token,
		dpp::interaction_response{ dpp::ir_channel_message_with_source, dpp::message{ msg }.set_flags(dpp::m_ephemeral) },
		std::move(callback)
	);
}

void mln::response::check_event_data_validity(const event_data_lite_t& event_data)
{
	if (event_data.creator == nullptr) [[unlikely]] {
		throw std::exception{ "Creator cannot be nullptr when making interaction response!" };
	}
	if (event_data.token.empty()) [[unlikely]] {
		throw std::exception{ "Token cannot be empty when making interaction response!" };
	}
	if (event_data.command_id == 0) [[unlikely]] {
		throw std::exception{ "Command id cannot be invalid when making interaction response!" };
	}
}
bool mln::response::is_event_data_valid(const event_data_lite_t& event_data) {
	return event_data.creator && event_data.command_id && !event_data.token.empty();
}
void mln::response::respond(event_data_lite_t& lite_data, std::string respond_msg, const bool always_log, std::string log_msg)
{
	mln::response::make_response(lite_data, std::move(respond_msg),
		mln::utility::get_conf_callback_is_error(lite_data, always_log, std::move(log_msg)));
}
void mln::response::respond(event_data_lite_t& lite_data, const dpp::message& respond_msg, const bool always_log, std::string log_msg)
{
	mln::response::make_response(lite_data, respond_msg,
		mln::utility::get_conf_callback_is_error(lite_data, always_log, std::move(log_msg)));
}

dpp::task<bool> mln::response::co_respond(event_data_lite_t& lite_data, std::string respond_msg, const bool always_log, std::string log_msg)
{
	co_return mln::utility::conf_callback_is_error(co_await mln::response::co_make_response(lite_data, std::move(respond_msg)), lite_data, always_log, std::move(log_msg));
}
dpp::task<bool> mln::response::co_respond(event_data_lite_t& lite_data, const dpp::message& respond_msg, const bool always_log, std::string log_msg)
{
	co_return mln::utility::conf_callback_is_error(co_await mln::response::co_make_response(lite_data, respond_msg), lite_data, always_log, std::move(log_msg));
}

void mln::response::think(event_data_lite_t& lite_data, const bool ephemeral, const bool always_log, std::string log_msg)
{
	mln::response::check_event_data_validity(lite_data);

	lite_data.is_first_reply = false;

	dpp::message msg{ lite_data.channel_id, std::string{"*"} };
	msg.guild_id = lite_data.guild_id;
	if (ephemeral) [[likely]] {
		msg.set_flags(dpp::m_ephemeral);
	}

	lite_data.creator->interaction_response_create(lite_data.command_id, lite_data.token, 
		dpp::interaction_response(dpp::interaction_response_type::ir_deferred_channel_message_with_source, std::move(msg)), 
		mln::utility::get_conf_callback_is_error(lite_data, always_log, std::move(log_msg)));
}

dpp::task<bool> mln::response::co_think(event_data_lite_t& lite_data, const bool ephemeral, const bool always_log, std::string log_msg)
{
	mln::response::check_event_data_validity(lite_data);

	lite_data.is_first_reply = false;

	dpp::message msg{ lite_data.channel_id, std::string{"*"} };
	msg.guild_id = lite_data.guild_id;
	if (ephemeral) [[likely]] {
		msg.set_flags(dpp::m_ephemeral);
	}

	co_return mln::utility::conf_callback_is_error(co_await lite_data.creator->co_interaction_response_create(lite_data.command_id, lite_data.token,
		dpp::interaction_response(dpp::interaction_response_type::ir_deferred_channel_message_with_source, std::move(msg))), 
		lite_data, always_log, std::move(log_msg));
}

