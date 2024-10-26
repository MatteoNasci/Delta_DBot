#include "utility/event_data_lite.h"

#include <dpp/discordclient.h>
#include <dpp/dispatcher.h>

#include <cstdint>
#include <string>
#include <type_traits>

mln::event_data_lite_t::event_data_lite_t() noexcept : event_data_lite_t{ 0, 0, 0, 0, 0, {}, {}, nullptr, true }
{
}

mln::event_data_lite_t::event_data_lite_t(event_data_lite_t&& rhs) noexcept :
	command_id{ rhs.command_id }, guild_id{ rhs.guild_id }, channel_id{ rhs.channel_id }, usr_id{ rhs.usr_id }, app_id{ rhs.app_id }, command_name{ std::move(rhs.command_name) }, token{ std::move(rhs.token) }, creator{ rhs.creator }, is_first_reply{ rhs.is_first_reply }
{
	rhs.command_name = {};
	rhs.token = {};
}

mln::event_data_lite_t::event_data_lite_t(const event_data_lite_t& rhs) noexcept : event_data_lite_t{ rhs.command_id, rhs.guild_id, rhs.channel_id, rhs.usr_id, rhs.app_id, rhs.command_name, rhs.token, rhs.creator, rhs.is_first_reply }
{
}

mln::event_data_lite_t& mln::event_data_lite_t::operator=(const event_data_lite_t& rhs) noexcept
{
	if (this != &rhs) {
		command_id = rhs.command_id;
		guild_id = rhs.guild_id;
		channel_id = rhs.channel_id;
		usr_id = rhs.usr_id;
		app_id = rhs.app_id;
		command_name = rhs.command_name;
		token = rhs.token;
		creator = rhs.creator;
		is_first_reply = rhs.is_first_reply;
	}

	return *this;
}

mln::event_data_lite_t& mln::event_data_lite_t::operator=(event_data_lite_t&& rhs) noexcept
{
	if (this != &rhs) {
		command_id = rhs.command_id;
		guild_id = rhs.guild_id;
		channel_id = rhs.channel_id;
		usr_id = rhs.usr_id;
		app_id = rhs.app_id;
		command_name = std::move(rhs.command_name);
		rhs.command_name = {};
		token = std::move(rhs.token);
		rhs.token = {};
		creator = rhs.creator;
		is_first_reply = rhs.is_first_reply;
	}

	return *this;
}

mln::event_data_lite_t::event_data_lite_t(const uint64_t command_id, const uint64_t guild_id, const uint64_t channel_id, const uint64_t usr_id, const uint64_t app_id, const std::string& command_name, const std::string& token, dpp::cluster* cluster, const bool is_first_reply) noexcept :
	command_id{ command_id }, guild_id{ guild_id }, channel_id{ channel_id }, usr_id{ usr_id }, app_id{ app_id }, command_name{ command_name }, token{ token }, creator{ cluster }, is_first_reply{ is_first_reply }
{
}

mln::event_data_lite_t::event_data_lite_t(const dpp::interaction_create_t* const event_data, const bool is_first_reply) noexcept : event_data_lite_t{}
{
	if (event_data) {
		command_id = event_data->command.id;
		guild_id = event_data->command.guild_id;
		channel_id = event_data->command.channel_id;
		usr_id = event_data->command.usr.id;
		app_id = event_data->command.application_id;
		command_name = event_data->command.get_command_name();
		token = event_data->command.token;
		creator = event_data->from ? event_data->from->creator : nullptr;
		this->is_first_reply = is_first_reply;
	}
}

mln::event_data_lite_t::event_data_lite_t(const dpp::interaction_create_t& event_data, const bool is_first_reply) noexcept :
	event_data_lite_t{ event_data.command.id, event_data.command.guild_id, event_data.command.channel_id, event_data.command.usr.id, event_data.command.application_id, event_data.command.get_command_name(), event_data.command.token, event_data.from ? event_data.from->creator : nullptr, is_first_reply}
{
}

mln::event_data_lite_t::event_data_lite_t(const dpp::interaction_create_t* const event_data, dpp::cluster& cluster, const bool is_first_reply) noexcept : event_data_lite_t{ event_data, is_first_reply }
{
	creator = &cluster;
}

mln::event_data_lite_t::event_data_lite_t(const dpp::interaction_create_t& event_data, dpp::cluster& cluster, const bool is_first_reply) noexcept : event_data_lite_t{ event_data, is_first_reply }
{
	creator = &cluster;
}
