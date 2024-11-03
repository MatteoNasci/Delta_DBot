#include "commands/base_action.h"
#include "commands/slash/base_slashcommand.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog.h"
#include "commands/slash/mog/mog_arma.h"
#include "commands/slash/mog/mog_cmd_data.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_help.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "commands/slash/mog/mog_team.h"
#include "database/database_handler.h"
#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"
#include "utility/caches.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>

#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

static const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> s_allowed_team_sub_commands{
        {"create", {0, mln::mog::mog_command_type::create}},
        {"delete", {0, mln::mog::mog_command_type::del}},
        {"join", {0, mln::mog::mog_command_type::join}},
        {"leave", {0, mln::mog::mog_command_type::leave}},
        {"leave_and_join", {0, mln::mog::mog_command_type::leave_and_join}},
        {"show", {0, mln::mog::mog_command_type::show}},
        {"help", {0, mln::mog::mog_command_type::help}}, };
static const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> s_allowed_arma_sub_commands{
        {"raw_cooldown", {1, mln::mog::mog_command_type::raw_cooldown}},
        {"cooldown", {1, mln::mog::mog_command_type::cooldown}},
        {"show_cooldowns", {1, mln::mog::mog_command_type::show_cooldowns}},
        {"start", {1, mln::mog::mog_command_type::start}},
        {"scheduled", {1, mln::mog::mog_command_type::scheduled}},
        {"help", {1, mln::mog::mog_command_type::help}}, };
static const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> s_allowed_help_sub_commands{
            {"generic", {2, mln::mog::mog_command_type::generic}}, };
static const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>> s_allowed_privacy_sub_commands{};
static const std::unordered_map<std::string, const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>>&> s_allowed_primary_sub_commands{
            {"team", s_allowed_team_sub_commands},
            {"arma", s_allowed_arma_sub_commands},
            {"help", s_allowed_help_sub_commands}, };

mln::mog::mog::mog(dpp::cluster& cluster, database_handler& in_database) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("mog"), "Commands related to the game MoG.", cluster.me.id)
        //Minimum permission required for using the commands
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        //team command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "team", "Collection of commands related to teams", false)
            //team create command
            .add_option(dpp::command_option(dpp::co_sub_command, "create", "Adds a new arma team", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the new arma team", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length())))
                .add_option(dpp::command_option(dpp::co_channel, "channel", "The channel dedicated to this team.", false)
                    .add_channel_type(dpp::channel_type::CHANNEL_TEXT))
                .add_option(dpp::command_option(dpp::co_role, "role", "The role associated with this team", false)))
            //team delete command
            .add_option(dpp::command_option(dpp::co_sub_command, "delete", "Removes an existing arma team", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to delete", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length()))))
            //team join coommand
            .add_option(dpp::command_option(dpp::co_sub_command, "join", "Add user to a team", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to join", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length())))
                .add_option(dpp::command_option(dpp::co_user, "user", "User to add to the team. If not set the command user will be added to the team.", false)))
            //team leave command
            .add_option(dpp::command_option(dpp::co_sub_command, "leave", "Remove user from a team", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to leave", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length())))
                .add_option(dpp::command_option(dpp::co_user, "user", "User to remove from the team. If not set the command user will be removed from the team.", false)))
            //team leave_and_join command
            .add_option(dpp::command_option(dpp::co_sub_command, "leave_and_join", "Remove user from a team", false)
                .add_option(dpp::command_option(dpp::co_string, "team_to_join", "The name of the arma team to join", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length())))
                .add_option(dpp::command_option(dpp::co_string, "team_to_leave", "The name of the arma team to leave", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length())))
                .add_option(dpp::command_option(dpp::co_user, "user", "Traget user. If not set the command user will be considered the target of the command.", false)))
            //team show command
            .add_option(dpp::command_option(dpp::co_sub_command, "show", "Displays data about a team or multiple teams", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to display", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length()))))
            //team help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the team group command", false)))
        //arma command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "arma", "Collection of commands related to arma", false)
            //arma start command
            .add_option(dpp::command_option(dpp::co_sub_command, "start", "Prepares a new arma session", false)
                .add_option(dpp::command_option(dpp::co_integer, "day", "The starting hour for the arma session (UTC).", true)
                    .set_min_value(1)
                    .set_max_value(31))
                .add_option(dpp::command_option(dpp::co_integer, "month", "The starting hour for the arma session (UTC).", true)
                    .set_min_value(1)
                    .set_max_value(12))
                .add_option(dpp::command_option(dpp::co_integer, "year", "The starting hour for the arma session (UTC).", true)
                    .set_min_value(static_cast<int64_t>(mln::constants::get_min_arma_year()))
                    .set_max_value(static_cast<int64_t>(mln::constants::get_max_arma_year())))
                .add_option(dpp::command_option(dpp::co_integer, "utc_hour", "The starting hour for the arma session (UTC).", true)
                    .set_min_value(0)
                    .set_max_value(23)))
            //arma scheduled command
            .add_option(dpp::command_option(dpp::co_sub_command, "scheduled", "Displays how long before next arma session", false)
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Determines if the result of the operation will be seen by everyone or only by the user", false)))
            //arma raw_cooldown command
            .add_option(dpp::command_option(dpp::co_sub_command, "raw_cooldown", "Allows a team member to update his arma cooldown", false)
                .add_option(dpp::command_option(dpp::co_integer, "minutes", "The user arma minutes cooldown.", true)
                    .set_min_value(static_cast<int64_t>(mln::constants::get_min_arma_cd_minutes()))
                    .set_max_value(static_cast<int64_t>(mln::constants::get_max_arma_cd_minutes())))
                .add_option(dpp::command_option(dpp::co_integer, "seconds", "The user arma seconds cooldown.", true)
                    .set_min_value(static_cast<int64_t>(mln::constants::get_min_arma_cd_seconds()))
                    .set_max_value(static_cast<int64_t>(mln::constants::get_max_arma_cd_seconds())))
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to update", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length()))))
            //arma cooldown command
            .add_option(dpp::command_option(dpp::co_sub_command, "cooldown", "Allows a team member to set his arma cooldown to 5 minutes", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to update", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length()))))
            //arma show_cooldowns command
            .add_option(dpp::command_option(dpp::co_sub_command, "show_cooldowns", "Create a persisting message that tracks a team cooldowns", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "The name of the arma team to track", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_team_name_length()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_team_name_length()))))
            //arma help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the arma group command", false)))
        //MoG help group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "help", "Gives generic information about the mog group command", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "generic", "Gives generic information about the mog group command", false)))
    ) }, 
    database{ in_database }, 
    commands{}
{
    commands[0] = std::make_unique<mln::mog::mog_team>(bot(), database);

    mln::mog::mog_team* team = dynamic_cast<mln::mog::mog_team*>(commands[0].get());
    if (team == nullptr) {
        throw std::exception("Failed to extract team pointer from mog commands!");
    }

    commands[1] = std::make_unique<mln::mog::mog_arma>(bot(), *team, "arma");
    commands[2] = std::make_unique<mln::mog::mog_help>(bot());

    cbot().log(dpp::loglevel::ll_debug, std::format("mog: [{}].", true));
}

dpp::job mln::mog::mog::command(dpp::slashcommand_t event_data)
{
    //Return error if event_data or cmd_data are incorrect
    //Data to be given to the selected command function
    mln::mog::mog_cmd_data_t cmd_data{};
    cmd_data.data = { event_data, bot(), true };

    if (!mln::response::is_event_data_valid(cmd_data.data)) {
        mln::utility::create_event_log_error(cmd_data.data, "Failed mog command, the event is incorrect!");
        co_return;
    }

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(cmd_data.data, "Failed mog command, discord error!", true, "Failed mog command, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    const dpp::command_interaction cmd_interaction = event_data.command.get_command_interaction();
    if (cmd_data.data.command_id == 0 || cmd_data.data.guild_id == 0 || cmd_data.data.channel_id == 0 || cmd_interaction.options.size() == 0) {
        static const std::string s_err_text = "Failed to proceed with mog command, event data is incorrect!";
        co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);
        co_return;
    }

    //Get the mapper for the primary cmds, return error if not found
    const dpp::command_data_option primary_cmd = cmd_interaction.options[0];
    cmd_data.data.command_name = std::format("{} {}", cmd_data.data.command_name, primary_cmd.name);
    const auto& it = s_allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == s_allowed_primary_sub_commands.end()) {
        const std::string err_text = std::format("Couldn't find primary sub command [{}].", primary_cmd.name);
        co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);

        co_return;
    }

    //Return error if primary_cmd is incorrect
    if (primary_cmd.options.size() == 0) {
        static const std::string s_err_text = "Failed to proceed with mog command, event data is incorrect!";
        co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);

        co_return;
    }

    //Get the sub_command handler, return an error if not found
    const std::unordered_map<std::string, std::tuple<size_t, mln::mog::mog_command_type>>& mapper = it->second;
    const dpp::command_data_option sub_command = primary_cmd.options[0];
    cmd_data.data.command_name = std::format("{} {}", cmd_data.data.command_name, sub_command.name);
    const auto& sub_it = mapper.find(sub_command.name);
    if (sub_it == mapper.end()) {
        const std::string err_text = std::format("Couldn't find [{}]'s sub command [{}].", primary_cmd.name, sub_command.name);
        co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);

        co_return;
    }

    //Return early if the selected mog command is not initialized correctly
    if (!mln::flags::has(commands[std::get<0>(sub_it->second)]->is_db_initialized(), db_saved_stmt_state::initialized)) {
        co_await mln::response::co_respond(cmd_data.data, "Failed database operation, the database was not initialized correctly!", true,
            std::format("Failed database operation, the database was not initialized correctly! Command: [{}].", cmd_data.data.command_name));
        co_return;
    }

    const mln::mog::mog_command_type cmd_type = std::get<1>(sub_it->second);
    const mln::mog::mog_init_type_flag init_requested = commands[std::get<0>(sub_it->second)]->get_requested_initialization_type(cmd_type);

    //If the command requests thinking, start thinking
    if (mln::flags::has(init_requested, mln::mog::mog_init_type_flag::thinking)) {
        static const std::string s_err_text = "Failed thinking confirmation, command aborted!";
        const bool is_error = co_await mln::response::co_think(cmd_data.data, true, false, {});
        if (is_error) {
            co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);
            co_return;
        }
    }

    //If the command requests cmd_data, retrieve it
    if (mln::flags::has(init_requested, mln::mog::mog_init_type_flag::cmd_data)) {

        //Prepare most common data for commands
        //Retrieve guild data
        const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_task(cmd_data.data.guild_id, cmd_data.data);
        if (!guild.has_value()) {
            co_return;
        }
        cmd_data.cmd_guild = guild.value();

        //Retrieve channel data
        const std::optional<std::shared_ptr<const dpp::channel>> channel = co_await mln::caches::get_channel_task(cmd_data.data.channel_id, cmd_data.data, &event_data.command.channel, &event_data.command.resolved.channels);
        if (!channel.has_value()) {
            co_return;
        }
        cmd_data.cmd_channel = channel.value();

        //Retrieve command user and bot information
        const std::optional<std::shared_ptr<const dpp::guild_member>> user = co_await mln::caches::get_member_task(cmd_data.data.guild_id, cmd_data.data.usr_id, cmd_data.data, &event_data.command.member, &event_data.command.resolved.members);
        if (!user.has_value()) {
            co_return;
        }
        cmd_data.cmd_usr = user.value();

        const std::optional<std::shared_ptr<const dpp::guild_member>> bot_opt = co_await mln::caches::get_member_task(cmd_data.data.guild_id, cmd_data.data.app_id, cmd_data.data, &event_data.command.member, &event_data.command.resolved.members);
        if (!bot_opt.has_value()) {
            co_return;
        }
        cmd_data.cmd_bot = bot_opt.value();

        //Retrieve user and bot perms, then return an error if the user and the bot don't have the required permissions
        const std::optional<dpp::permission> user_perm = co_await mln::perms::get_computed_permission_task(cmd_data.cmd_guild->owner_id, *(cmd_data.cmd_channel), *(cmd_data.cmd_usr), cmd_data.data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
        if (!user_perm.has_value()) {
            co_return;
        }
        cmd_data.cmd_usr_perm = user_perm.value();

        const std::optional<dpp::permission> bot_perm = co_await mln::perms::get_computed_permission_task(cmd_data.cmd_guild->owner_id, *(cmd_data.cmd_channel), *(cmd_data.cmd_bot), cmd_data.data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
        if (!bot_perm.has_value()) {
            co_return;
        }
        cmd_data.cmd_bot_perm = bot_perm.value();
    }

    co_await commands[std::get<0>(sub_it->second)]->command(event_data, cmd_data, cmd_type);
}

std::optional<std::function<void()>> mln::mog::mog::job(dpp::slashcommand_t event_data)
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::mog::mog::use_job() const noexcept
{
    return false;
}
