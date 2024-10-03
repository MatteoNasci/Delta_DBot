#include "commands/base_action.h"
#include "commands/slash/base_slashcommand.h"
#include "commands/slash/mog/mog.h"
#include "utility/response.h"
#include "utility/event_data_lite.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/permissions.h>

#include <functional>
#include <optional>
#include <type_traits>

mln::mog::mog::mog(dpp::cluster& cluster, database_handler& in_database) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("mog"), "Commands related to the game MoG.", cluster.me.id)
        //Minimum permission required for using the commands
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        //team command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "team", "Collection of commands related to teams", false)
            //team create command
            .add_option(dpp::command_option(dpp::co_sub_command, "create", "Adds a new arma team", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the new arma team", true))
                .add_option(dpp::command_option(dpp::co_channel, "team channel", "The channel dedicated to this team.", true)
                    .add_channel_type(dpp::channel_type::CHANNEL_TEXT))
                .add_option(dpp::command_option(dpp::co_role, "team role", "The role associated with this team", false)))
            //team delete command
            .add_option(dpp::command_option(dpp::co_sub_command, "delete", "Removes an existing arma team", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the arma team to delete", true)))
            //team join coommand
            .add_option(dpp::command_option(dpp::co_sub_command, "join", "Add user to a team", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the arma team to join", true))
                .add_option(dpp::command_option(dpp::co_user, "user", "User to add to the team. If not set the command user will be added to the team.", false)))
            //team leave command
            .add_option(dpp::command_option(dpp::co_sub_command, "leave", "Remove user from a team", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the arma team to leave", true))
                .add_option(dpp::command_option(dpp::co_user, "user", "User to remove from the team. If not set the command user will be removed from the team.", false)))
            //team show command
            .add_option(dpp::command_option(dpp::co_sub_command, "show", "Displays data about a team", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the arma team to display", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "If set to True displays the results to everyone. False by default.", false)))
            //team help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the team group command", false)))
        //arma commmand group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "arma", "Collection of commands related to arma", false)
            //arma start command
            .add_option(dpp::command_option(dpp::co_sub_command, "start", "Schedules the next 1 hour arma session", false)
                .add_option(dpp::command_option(dpp::co_string, "date", "The date arma starts. The accepted format (UTC) is: dd:MM hh:mm:ss  (M = month, m = minute)", true)))
            //arma cooldown command
            .add_option(dpp::command_option(dpp::co_sub_command, "cooldown", "Allows a team member to update his arma cooldown", false)
                .add_option(dpp::command_option(dpp::co_string, "cd", "The user arma cooldown. The accepted format is: m:ss . Example: 3:24", true)))
            //arma track_cooldowns command
            .add_option(dpp::command_option(dpp::co_sub_command, "track_cooldowns", "Create a persisting message that tracks a team cooldowns", false)
                .add_option(dpp::command_option(dpp::co_string, "team name", "The name of the arma team to track", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "If set to True displays the results to everyone. False by default.", false)))
            //arma help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the arma group command", false)))
        //MoG help group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "help", "Gives generic information about the mog group command", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "generic", "Gives generic information about the mog group command", false)))
    ) }, database{ in_database } {}

dpp::job mln::mog::mog::command(dpp::slashcommand_t event_data) const
{
    mln::event_data_lite_t lite_data{ event_data, bot(), true };
    co_await mln::response::co_respond(lite_data, "Commands not implemented yet!", false, {});
    co_return;
    /*static const std::unique_ptr<mln::mog::base_mog_command> commands[]{
        
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_insert_sub_commands{
        {"url", {commands[0], mln::db_command_type::url}},
        {"file", {commands[0], mln::db_command_type::file}},
        {"text", {commands[0], mln::db_command_type::text}},
        {"help", {commands[0], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_update_sub_commands{
        {"description", {commands[5], mln::db_command_type::description}},
        {"nsfw", {commands[5], mln::db_command_type::nsfw}},
        {"help", {commands[5], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_delete_sub_commands{
        {"single", {commands[4], mln::db_command_type::single}},
        {"user", {commands[4], mln::db_command_type::user}},
        {"self", {commands[4], mln::db_command_type::self}},
        {"guild", {commands[4], mln::db_command_type::guild}},
        {"help", {commands[4], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_config_sub_commands{
        {"update_dump_channel", {commands[1], mln::db_command_type::update_dump_channel}},
        {"help", {commands[1], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_select_sub_commands{
        {"single", {commands[2], mln::db_command_type::single}},
        {"help", {commands[2], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_show_sub_commands{
        {"all", {commands[3], mln::db_command_type::all}},
        {"user", {commands[3], mln::db_command_type::user}},
        {"help", {commands[3], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_help_sub_commands{
        {"generic", {commands[6], mln::db_command_type::generic}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_privacy_sub_commands{
        {"policy", {commands[7], mln::db_command_type::policy}},
    };
    static const std::unordered_map<std::string, const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>>&> s_allowed_primary_sub_commands{
        {"insert", s_allowed_insert_sub_commands},
        {"config", s_allowed_config_sub_commands},
        {"select", s_allowed_select_sub_commands},
        {"show", s_allowed_show_sub_commands},
        {"delete", s_allowed_delete_sub_commands},
        {"update", s_allowed_update_sub_commands},
        {"help", s_allowed_help_sub_commands},
        {"privacy", s_allowed_privacy_sub_commands},
    };

    //Return error if event_data or cmd_data are incorrect
    //Data to be given to the selected command function
    db_cmd_data_t cmd_data{ { event_data, bot(), true }, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0 };

    if (!mln::response::is_event_data_valid(cmd_data.data)) {
        mln::utility::create_event_log_error(cmd_data.data, "Failed db command, the event is incorrect!");
        co_return;
    }

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(cmd_data.data, "Failed db command, discord error!", true, "Failed db command, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    const dpp::command_interaction cmd_interaction = event_data.command.get_command_interaction();
    if (cmd_data.data.command_id == 0 || cmd_data.data.guild_id == 0 || cmd_data.data.channel_id == 0 || cmd_interaction.options.size() == 0) {
        static const std::string s_err_text = "Failed to proceed with db command, event data is incorrect!";
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
        static const std::string s_err_text = "Failed to proceed with db command, event data is incorrect!";
        co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);

        co_return;
    }

    //Get the sub_command handler, return an error if not found
    const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>>& mapper = it->second;
    const dpp::command_data_option sub_command = primary_cmd.options[0];
    cmd_data.data.command_name = std::format("{} {}", cmd_data.data.command_name, sub_command.name);
    const auto& sub_it = mapper.find(sub_command.name);
    if (sub_it == mapper.end()) {
        const std::string err_text = std::format("Couldn't find [{}]'s sub command [{}].", primary_cmd.name, sub_command.name);
        co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);

        co_return;
    }

    //Return early if the selected db command is not initialized correctly
    if (!(std::get<0>(sub_it->second)->is_db_initialized())) {
        co_await mln::response::co_respond(cmd_data.data, "Failed database operation, the database was not initialized correctly!", true,
            std::format("Failed database operation, the database was not initialized correctly! Command: [{}].", sub_command.name));
        co_return;
    }

    const db_command_type cmd_type = std::get<1>(sub_it->second);
    const db_init_type_flag init_requested = std::get<0>(sub_it->second)->get_requested_initialization_type(cmd_type);

    //If the command requests thinking, start thinking
    const bool is_thinking = (init_requested & db_init_type_flag::thinking) != db_init_type_flag::none;
    if (is_thinking) {
        static const std::string s_err_text = "Failed thinking confirmation, command aborted!";
        const bool is_error = co_await mln::response::co_think(cmd_data.data, true, false, {});
        if (is_error) {
            co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);
            co_return;
        }
    }

    //If the command requests cmd_data, retrieve it
    if ((init_requested & db_init_type_flag::cmd_data) != db_init_type_flag::none) {

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

    //If requested, determine the dump channel
    if ((init_requested & mln::db_init_type_flag::dump_channel) != db_init_type_flag::none) {

        //Recover dump channel id from cache if possible. Default value for dump channel is the channel the command was invoked in
        uint64_t dump_channel_id{ cmd_data.data.channel_id };
        const std::optional<uint64_t> opt_channel = mln::caches::get_dump_channel_id(cmd_data.data.guild_id);
        if (!opt_channel.has_value()) {

            static const std::string s_err_text = "Failed to retrieve dump channel, either an internal error occurred or the guild from which this command was called is not registered in the database!";
            co_await mln::response::co_respond(cmd_data.data, s_err_text, true, s_err_text);

            co_return;
        }

        if (opt_channel.value() != 0) {
            dump_channel_id = opt_channel.value();
        }

        //Get the full channel data from cache if possible
        if (dump_channel_id == cmd_data.data.channel_id && cmd_data.cmd_channel) {
            cmd_data.dump_channel = cmd_data.cmd_channel;
        }
        else {
            const std::optional<std::shared_ptr<const dpp::channel>> dump_channel_opt = co_await mln::caches::get_channel_task(dump_channel_id, cmd_data.data, &event_data.command.channel, &event_data.command.resolved.channels);
            if (!dump_channel_opt.has_value()) {
                co_return;
            }
            cmd_data.dump_channel = dump_channel_opt.value();
        }

        //If the cmd bot and cmd guild data has been filled, get the dump channel bot perms
        if (cmd_data.cmd_bot && cmd_data.cmd_guild) {
            const std::optional<dpp::permission> bot_dump_channel_perm = co_await mln::perms::get_computed_permission_task(cmd_data.cmd_guild->owner_id, *(cmd_data.dump_channel), *(cmd_data.cmd_bot), cmd_data.data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
            if (!bot_dump_channel_perm.has_value()) {
                co_return;
            }
            cmd_data.dump_channel_bot_perm = bot_dump_channel_perm.value();
        }
    }

    co_await std::get<0>(sub_it->second)->command(event_data, cmd_data, cmd_type);*/
}

std::optional<std::function<void()>> mln::mog::mog::job(dpp::slashcommand_t event_data) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::mog::mog::use_job() const
{
    return false;
}
