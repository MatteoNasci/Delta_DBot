#include "commands/base_action.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_arma.h"
#include "commands/slash/mog/mog_cmd_data.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "commands/slash/mog/mog_team.h"
#include "commands/slash/mog/mog_team_data.h"
#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"
#include "utility/constants.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

static constexpr size_t s_pagination_timeout = 120;
static constexpr size_t s_pagination_max_text_size = 2000;

mln::mog::mog_arma::mog_arma(dpp::cluster& cluster, mln::mog::mog_team& teams_handler) : base_mog_command{ cluster }, teams_handler{ teams_handler }
{
	cbot().log(dpp::loglevel::ll_debug, std::format("mog_arma: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

dpp::task<void> mln::mog::mog_arma::command(const dpp::slashcommand_t& event_data, mln::mog::mog_cmd_data_t& cmd_data, const mln::mog::mog_command_type type)
{
	switch (type) {
	case mln::mog::mog_command_type::cooldown:
		co_await mln::mog::mog_arma::cooldown(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::show_cooldowns:
		co_await mln::mog::mog_arma::show_cooldowns(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::help:
		co_await mln::mog::mog_arma::help(cmd_data);
		break;
	default:
		co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
			std::format("Failed command, the given sub_command [{}] is not supported for /mog arma!", mln::mog::get_cmd_type_text(type)));
		break;
	}
}

mln::mog::mog_init_type_flag mln::mog::mog_arma::get_requested_initialization_type(const mln::mog::mog_command_type cmd) const noexcept
{
	switch (cmd) {
	case mln::mog::mog_command_type::cooldown:
	case mln::mog::mog_command_type::show_cooldowns:
		return mln::flags::add(mln::mog::mog_init_type_flag::cmd_data, mln::mog::mog_init_type_flag::thinking);
	case mln::mog::mog_command_type::help:
		return mln::mog::mog_init_type_flag::none;
	default:
		return mln::mog::mog_init_type_flag::all;
	}
}

mln::db_saved_stmt_state mln::mog::mog_arma::is_db_initialized() const noexcept
{
	return db_saved_stmt_state::initialized;
}

dpp::task<void> mln::mog::mog_arma::cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
	const dpp::command_value& min_param = event_data.get_parameter("minutes");
	const dpp::command_value& sec_param = event_data.get_parameter("seconds");

	if (!std::holds_alternative<int64_t>(min_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! Failed to retrieve the minutes value!", true, {});
		co_return;
	}

	const int64_t mins = std::get<int64_t>(min_param);
	if (mins < mln::constants::get_min_arma_cd_minutes() || mins > mln::constants::get_max_arma_cd_minutes()) {
		co_await mln::response::co_respond(cmd_data.data, 
			std::format("Failed to update cooldown! The retrieved minutes value is outside the valid range of [{}/{}]!", 
			mln::constants::get_min_arma_cd_minutes(), mln::constants::get_max_arma_cd_minutes()), 
			false, {});
		co_return;
	}

	if (!std::holds_alternative<int64_t>(sec_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! Failed to retrieve the seconds value!", true, {});
		co_return;
	}

	const int64_t secs = std::get<int64_t>(sec_param);
	if (mins < mln::constants::get_min_arma_cd_seconds() || mins > mln::constants::get_max_arma_cd_seconds()) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to update cooldown! The retrieved seconds value is outside the valid range of [{}/{}]!",
				mln::constants::get_min_arma_cd_seconds(), mln::constants::get_max_arma_cd_seconds()),
			false, {});
		co_return;
	}

	const uint64_t total_cd_delay = static_cast<uint64_t>(secs + (mins * 60));

	const dpp::command_value& name_param = event_data.get_parameter("name");

	const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};
	const mln::utility::text_validity_t validity_data{
		.can_be_null = true,
		.log_if_null = false,
		.can_be_empty = true,
		.log_if_empty = false,
		.log_if_out_of_bounds = true,
		.min_size = mln::constants::get_min_team_name_length(),
		.max_size = mln::constants::get_max_team_name_length() };
	if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, validity_data, "team name"))) {

		co_return;
	}

	const size_t teams_with_user = teams_handler.teams_with_user(cmd_data.data.guild_id, cmd_data.data.usr_id);
	
	switch (teams_with_user) {
	case 0:
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of any team!", false, {});
		co_return;
	case 1:
		break;
	default:
		if (!teams_handler.is_user_in_team(cmd_data.data.guild_id, cmd_data.data.usr_id, name)) {
			const std::string err_text = std::format("Failed to update cooldown! The command user is part of [{}] teams but the name parameter [{}] was either not set or the user is not part of the associated team!", teams_with_user, name);
			co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);
			co_return;
		}
		break;
	}

	const std::optional<mln::mog::mog_team_data_t> team_data = name.empty() ? teams_handler.get_team(cmd_data.data.guild_id, cmd_data.data.usr_id) : teams_handler.get_team(cmd_data.data.guild_id, name);
	if (!team_data.has_value()) {
		const std::string err_text = name.empty() ? "Failed to update cooldown! The command user is not part of any team!" : "Failed to update cooldown! The command user is not part of the given team!";
		co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);
		co_return;
	}

	for (const mln::mog::mog_team_data_t::user_data_t& u_data : team_data.value().users_id_cd) {
		if (u_data.id == cmd_data.data.usr_id) {
			const uint64_t creation_time = static_cast<uint64_t>(event_data.command.id.get_creation_time());

			mln::mog::mog_team_data_t::user_data_t to_set;
			to_set.id = cmd_data.data.usr_id;
			to_set.cd = creation_time + total_cd_delay;
			to_set.last_update = creation_time;
			
			if (!teams_handler.set_user_cooldown(cmd_data.data.guild_id, team_data.value().name, to_set)) {
				co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of the given team!", true, "Failed to update cooldown! The command user is not part of the given team! (cooldown)");
				co_return;
			}

			co_await mln::response::co_respond(cmd_data.data, "Cooldown updated!", false, {});
			co_return;
		}
	}

	co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of the given team!", true, "Failed to update cooldown! The command user is no longer part of the given team! (over)");
}

dpp::task<void> mln::mog::mog_arma::show_cooldowns(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
	const dpp::command_value& name_param = event_data.get_parameter("name");

	const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};
	const mln::utility::text_validity_t validity_data{
		.can_be_null = true,
		.log_if_null = false,
		.can_be_empty = true,
		.log_if_empty = false,
		.log_if_out_of_bounds = true,
		.min_size = mln::constants::get_min_team_name_length(),
		.max_size = mln::constants::get_max_team_name_length() };
	if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, validity_data, "team name"))) {

		co_return;
	}
	const bool show_all_teams = name.empty();

	std::vector<mln::mog::mog_team_data_t> data_to_display{};
	if (show_all_teams) {
		teams_handler.get_teams(cmd_data.data.guild_id, data_to_display);
	}
	else {
		const std::optional<mln::mog::mog_team_data_t> team = teams_handler.get_team(cmd_data.data.guild_id, name);
		if (team.has_value()) {
			data_to_display.push_back(team.value());
		}
	}

	if (data_to_display.empty()) {
		static const std::string s_err_text = "Failed to show teams cooldown data, no team associated with the server!";

		co_await mln::response::co_respond(cmd_data.data, show_all_teams ? s_err_text : std::format("Failed to show teams cooldown data, no team named [{}] found associated with the server!", name), false, {});
		co_return;
	}

	std::vector<std::string> records{};
	records.reserve(data_to_display.size());
	size_t total_size = 0;
	for (const mln::mog::mog_team_data_t& team : data_to_display) {
		records.emplace_back(mln::mog::mog_team_data_t::to_string(team));
		total_size += records[records.size() - 1].size();
	}

	if (total_size > mln::constants::get_max_characters_reply_msg()) {
		co_await mln::response::co_respond(cmd_data.data, "Proceeding with pagination...", false, {});

		mln::utility::manage_paginated_embed(mln::utility::paginated_data_t{
			.event_data = cmd_data.data,
			.time_limit_seconds = s_pagination_timeout,
			.text_limit = s_pagination_max_text_size }, std::make_shared<const std::vector<std::string>>(std::move(records)));
	}
	else {
		std::string msg{};
		for (size_t i = 0; i < records.size(); ++i) {
			msg = std::format("{}{}", msg, std::move(records[i]));
		}

		co_await mln::response::co_respond(cmd_data.data, msg, false, {});
	}
}

dpp::task<void> mln::mog::mog_arma::help(mog_cmd_data_t& cmd_data) const
{
	static const dpp::message s_info = dpp::message{ "Information regarding the `/mog arma` commands..." }
		.set_flags(dpp::m_ephemeral)
		.add_embed(dpp::embed{}.set_description(R"""(The `/mog arma` set of commands exists to manage cooldowns and other aspects of MOG's Armageddon event. These commands require at least one existing MOG team.

**Types of commands:**

- **/mog arma cooldown**  
  *Parameters:* minutes[integer, required], seconds[integer, required], name[text, optional].  
  This command requires specifying the `minutes` (between 0 and 5) and `seconds` (between 0 and 59), along with an optional `name` parameter.
  A new cooldown for the command user will be set (on the specified team) based on the given minutes and seconds. This cooldown can be viewed using the `/mog arma show_cooldowns` command.
  If the user is part of only one MOG team, the optional parameter `name` can be ignored. Otherwise, it MUST be set to a valid team name that includes the user.

- **/mog arma show_cooldowns**  
  *Parameters:* name[text, optional].  
  This command takes an optional `name` parameter associated with a team. If not set, the command will display cooldown data for all MOG teams on the server. If set, it will show cooldown data only for the specified team.
  
  WARNING: A message created with this command will not update itself if someone updates their own cooldown, so it's a good idea to refresh this command if you are waiting for someone's updated cooldown.)"""));

	co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the mog arma help text!");
	co_return;
}
