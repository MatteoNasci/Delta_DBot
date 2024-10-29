#include "commands/slash/mog/mog_team_data.h"

#include <dpp/channel.h>
#include <dpp/role.h>
#include <dpp/user.h>

#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

mln::mog::mog_team_data_t::mog_team_data_t() : name{}, guild_id{ 0 }, channel_id{ 0 }, role_id{ 0 }, users_id_cd{}
{
}

mln::mog::mog_team_data_t::mog_team_data_t(std::string name, const uint64_t guild_id, const uint64_t channel_id, const uint64_t role_id) :
	name{ std::move(name) }, guild_id{ guild_id }, channel_id{ channel_id }, role_id{ role_id }, users_id_cd{}
{
}

mln::mog::mog_team_data_t::user_data_t::user_data_t() noexcept : id{ 0 }, cd{ 0 }, last_update{ 0 }
{
}

mln::mog::mog_team_data_t::user_data_t::user_data_t(const uint64_t id, const uint64_t cd, const uint64_t last_update) noexcept : id{ id }, cd{ cd }, last_update{ last_update }
{
}

std::string mln::mog::mog_team_data_t::to_string(const mog_team_data_t& data, const uint64_t min_valid_cd, const uint64_t min_cd_clamp)
{
	std::string result = mln::mog::mog_team_data_t::to_string_partial(data);

	//<t:time:R> for relative time display. https://discordtimestamp.com/
	//<t:time:T> for long time display

	uint64_t min_cd = std::numeric_limits<uint64_t>::max(), max_cd = 0;
	for (const mln::mog::mog_team_data_t::user_data_t& u_data : data.users_id_cd) {
		const bool is_cd_valid = mln::mog::mog_team_data_t::is_member_cooldown_valid(u_data, min_valid_cd);

		if (is_cd_valid) [[likely]] {
			const uint64_t clamped_cd = u_data.cd < min_cd_clamp ? min_cd_clamp : u_data.cd;
			if (min_cd > clamped_cd) {
				min_cd = clamped_cd;
			}
			if (max_cd < clamped_cd) {
				max_cd = clamped_cd;
			}

			result = std::format("{}\t[{}], cooldown: [<t:{}:R>, at <t:{}:T>].\t\tLast updated: [<t:{}:R>].\n",
				result,
				dpp::user::get_mention(u_data.id),
				u_data.cd,
				u_data.cd,
				u_data.last_update);
		}else{
			result = std::format("{}\t[{}], cooldown: [{}, at {}].\t\tLast updated: [{}].\n",
				result,
				(u_data.id != 0 ? dpp::user::get_mention(u_data.id) : "Unknown user"),
				(u_data.cd != 0 ? std::format("<t:{}:R>", u_data.cd) : "Null"),
				(u_data.cd != 0 ? std::format("<t:{}:T>", u_data.cd) : "Null"),
				(u_data.last_update != 0 ? std::format("<t:{}:R>", u_data.last_update) : "Null"));
		}
	}

	return std::format("{}\tMax cooldown difference: [{}].\n", result, (max_cd != 0 ? (max_cd - min_cd) : 0));
}

std::string mln::mog::mog_team_data_t::to_string_partial(const mog_team_data_t& data)
{
	return std::format("\nTeam: [{}]. Channel: [{}], Role: [{}]. Team members count: [{}].\n",
		data.name, data.channel_id != 0 ? dpp::channel::get_mention(data.channel_id) : "None", data.role_id != 0 ? dpp::role::get_mention(data.role_id) : "None", data.users_id_cd.size());
}

std::string mln::mog::mog_team_data_t::to_string_no_cd(const mog_team_data_t& data)
{
	std::string result = mln::mog::mog_team_data_t::to_string_partial(data);
	
	for (const mln::mog::mog_team_data_t::user_data_t& u_data : data.users_id_cd) {
		result = std::format("{}\t[{}]\n", result, dpp::user::get_mention(u_data.id));
	}

	return result;
}

bool mln::mog::mog_team_data_t::is_member_cooldown_valid(const user_data_t& user, const uint64_t min_valid_cd) noexcept
{
	return user.id != 0 && user.cd != 0 && user.cd >= min_valid_cd;
}
