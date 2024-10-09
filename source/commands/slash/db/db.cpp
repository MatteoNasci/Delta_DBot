#include "commands/slash/base_slashcommand.h"
#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_config.h"
#include "commands/slash/db/db_delete.h"
#include "commands/slash/db/db_help.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "commands/slash/db/db_insert.h"
#include "commands/slash/db/db_privacy.h"
#include "commands/slash/db/db_select.h"
#include "commands/slash/db/db_show.h"
#include "commands/slash/db/db_update.h"
#include "database/database_handler.h"
#include "utility/caches.h"
#include "utility/constants.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/permissions.h>

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

mln::db::db(dpp::cluster& cluster, database_handler& in_database) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("db"), "Manage the database.", cluster.me.id)
        //Minimum permission required for using the commands
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        //Config command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "config", "Collection of commands used to enhance the database features", false)
            //Config update_dump_channel command
            .add_option(dpp::command_option(dpp::co_sub_command, "update_dump_channel", "Updates the dump channel used for this bot in the current server!", false)
                .add_option(dpp::command_option(dpp::co_channel, "channel", "The channel dedicated to this bot.", false)
                    .add_channel_type(dpp::channel_type::CHANNEL_TEXT)))
            //Config help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the config group command", false)))
        //Insert command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "insert", "Insert a new record in the database", false)
            //Insert url command
            .add_option(dpp::command_option(dpp::co_sub_command, "url", "Insert a new record from an url", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_string, "url", "Identifying url for the discord message/attachment", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_url()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_url())))
                .add_option(dpp::command_option(dpp::co_boolean, "nsfw", "Set this to True if the content inserted is nsfw", true))
                .add_option(dpp::command_option(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Insert file command
            .add_option(dpp::command_option(dpp::co_sub_command, "file", "Insert a new record from a given file", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_attachment, "file", "The attachment to add to the record", true))
                .add_option(dpp::command_option(dpp::co_boolean, "nsfw", "Set this to True if the content inserted is nsfw", true))
                .add_option(dpp::command_option(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description())))
                .add_option(dpp::command_option(dpp::co_attachment, "file1", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file2", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file3", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file4", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file5", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file6", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file7", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file8", "The attachment to add to the record", false))
                .add_option(dpp::command_option(dpp::co_attachment, "file9", "The attachment to add to the record", false)))
            //Insert text command
            .add_option(dpp::command_option(dpp::co_sub_command, "text", "Insert a new record from a given text", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_boolean, "nsfw", "Set this to True if the content inserted is nsfw", true))
                .add_option(dpp::command_option(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Insert help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the insert group command", false)))
        //Update command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "update", "Updates a database record", false)
            //Update description command
            .add_option(dpp::command_option(dpp::co_sub_command, "description", "Updates a database record's description", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description())))
                .add_option(dpp::command_option(dpp::co_user, "owner", "Fill this parameter in case you want to update a record owned by someone else", false)))
            //Update nsfw command
            .add_option(dpp::command_option(dpp::co_sub_command, "nsfw", "Updates a database record's nsfw tag", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_boolean, "nsfw", "Set this to True if the content updated is nsfw", true))
                .add_option(dpp::command_option(dpp::co_user, "owner", "Fill this parameter in case you want to update a record owned by someone else", false)))
            //Update help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the update group command", false)))
        //Delete command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "delete", "Deletes a database record", false)
            //Delete guild command
            .add_option(dpp::command_option(dpp::co_sub_command, "guild", "Deletes all records for the guild", false))
            //Delete self command
            .add_option(dpp::command_option(dpp::co_sub_command, "self", "Deletes all records related to the user from the entire db (other servers included)", false))
            //Delete user command
            .add_option(dpp::command_option(dpp::co_sub_command, "user", "Deletes all records from the given user for the guild", false)
                .add_option(dpp::command_option(dpp::co_user, "user", "Select for deletion only the records created by this user", true)))
            //Delete single command
            .add_option(dpp::command_option(dpp::co_sub_command, "single", "Deletes one record for the guild", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record to delete", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_user, "owner", "Fill this parameter in case you want to delete a record owned by someone else", false)))
            //Delete help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the delete command group", false)))
        //Select command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "select", "Selects and returns a single database record", false)
            //Select single command
            .add_option(dpp::command_option(dpp::co_sub_command, "single", "Selects and returns one record", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Determines if the result of the operation will be seen by everyone or only by the user", false)))
            //Select help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the select command group", false)))
        //Show command group
        .add_option(dpp::command_option(dpp::co_sub_command_group, "show", "Selects and returns several records' info", false)
            //Show all command
            .add_option(dpp::command_option(dpp::co_sub_command, "all", "Selects and returns all records' info", false))
            //Show user command
            .add_option(dpp::command_option(dpp::co_sub_command, "user", "Selects and returns all user records' info", false)
                .add_option(dpp::command_option(dpp::co_user, "user", "The database will show all the records created by this user", true)))
            //Show help command
            .add_option(dpp::command_option(dpp::co_sub_command, "help", "Gives detailed information about the show command group", false)))
        //db help command
        .add_option(dpp::command_option(dpp::co_sub_command_group, "help", "Gives generic information about the db group command", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "generic", "Gives generic information about the db group command", false)))
        //db privacy command
        .add_option(dpp::command_option(dpp::co_sub_command_group, "privacy", "Gives information about the db group command privacy policy", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "policy", "Gives information about the db group command privacy policy", false)))
    ) }, 
    database{ in_database }, commands{}, 
    allowed_insert_sub_commands{ 
        {"url", {0, mln::db_command_type::url}},
        {"file", {0, mln::db_command_type::file}},
        {"text", {0, mln::db_command_type::text}},
        {"help", {0, mln::db_command_type::help}}, }, 
    allowed_update_sub_commands{ 
        {"description", {5, mln::db_command_type::description}},
        {"nsfw", {5, mln::db_command_type::nsfw}},
        {"help", {5, mln::db_command_type::help}}, },
    allowed_delete_sub_commands{ 
        {"single", {4, mln::db_command_type::single}},
        {"user", {4, mln::db_command_type::user}},
        {"self", {4, mln::db_command_type::self}},
        {"guild", {4, mln::db_command_type::guild}},
        {"help", {4, mln::db_command_type::help}}, },
    allowed_config_sub_commands{ 
        {"update_dump_channel", {1, mln::db_command_type::update_dump_channel}},
        {"help", {1, mln::db_command_type::help}}, },
    allowed_select_sub_commands{ 
        {"single", {2, mln::db_command_type::single}},
        {"help", {2, mln::db_command_type::help}}, },
    allowed_show_sub_commands{ 
        {"all", {3, mln::db_command_type::all}},
        {"user", {3, mln::db_command_type::user}},
        {"help", {3, mln::db_command_type::help}}, },
    allowed_help_sub_commands{ 
        {"generic", {6, mln::db_command_type::generic}}, },
    allowed_privacy_sub_commands{ 
        {"policy", {7, mln::db_command_type::policy}}, },
    allowed_primary_sub_commands{ 
        {"insert", allowed_insert_sub_commands},
        {"config", allowed_config_sub_commands},
        {"select", allowed_select_sub_commands},
        {"show", allowed_show_sub_commands},
        {"delete", allowed_delete_sub_commands},
        {"update", allowed_update_sub_commands},
        {"help", allowed_help_sub_commands},
        {"privacy", allowed_privacy_sub_commands}, }
    {
        commands[0] = std::make_unique<mln::db_insert>(bot(), database);
        commands[1] = std::make_unique<mln::db_config>(bot(), database);
        commands[2] = std::make_unique<mln::db_select>(bot(), database);
        commands[3] = std::make_unique<mln::db_show>(bot(), database);
        commands[4] = std::make_unique<mln::db_delete>(bot(), database);
        commands[5] = std::make_unique<mln::db_update>(bot(), database);
        commands[6] = std::make_unique<mln::db_help>(bot());
        commands[7] = std::make_unique<mln::db_privacy>(bot());
    }

dpp::job mln::db::command(dpp::slashcommand_t event_data) const{
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
    const auto& it = allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == allowed_primary_sub_commands.end()) {
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
    const std::unordered_map<std::string, std::tuple<size_t, db_command_type>>& mapper = it->second;
    const dpp::command_data_option sub_command = primary_cmd.options[0];
    cmd_data.data.command_name = std::format("{} {}", cmd_data.data.command_name, sub_command.name);
    const auto& sub_it = mapper.find(sub_command.name);
    if (sub_it == mapper.end()) {
        const std::string err_text = std::format("Couldn't find [{}]'s sub command [{}].", primary_cmd.name, sub_command.name);
        co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);

        co_return;
    }

    //Return early if the selected db command is not initialized correctly
    if (!(commands[std::get<0>(sub_it->second)]->is_db_initialized())) {
        co_await mln::response::co_respond(cmd_data.data, "Failed database operation, the database was not initialized correctly!", true,
            std::format("Failed database operation, the database was not initialized correctly! Command: [{}].", cmd_data.data.command_name));
        co_return;
    }

    const db_command_type cmd_type = std::get<1>(sub_it->second);
    const db_init_type_flag init_requested = commands[std::get<0>(sub_it->second)]->get_requested_initialization_type(cmd_type);

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
        uint64_t dump_channel_id{cmd_data.data.channel_id};
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
        } else {
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

    co_await commands[std::get<0>(sub_it->second)]->command(event_data, cmd_data, cmd_type);
}

std::optional<std::function<void()>> mln::db::job(dpp::slashcommand_t) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::db::use_job() const
{
    return false;
}
