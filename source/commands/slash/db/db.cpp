#include "commands/slash/db/db.h"
#include "bot_delta.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "commands/slash/db/db_update_dump_channel.h"
#include "commands/slash/db/db_insert.h"
#include "commands/slash/db/db_select.h"
#include "commands/slash/db/db_show.h"
#include "commands/slash/db/db_delete.h"
#include "commands/slash/db/db_update.h"
#include "commands/slash/db/db_help.h"
#include "commands/slash/db/db_privacy.h"
#include "utility/caches.h"

#include <dpp/queues.h>

#include <memory>

typedef dpp::command_option cmd_opt;
typedef dpp::command_option_range cmd_opt_r;

mln::db::db(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("db", "Manage the database.", delta->bot.me.id)
        //Minimum permission required for using the commands
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        //Setup command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "setup", "Collection of commands used to enhance the database features", false)
            //Setup update_dump_channel command
            .add_option(cmd_opt(dpp::co_sub_command, "update_dump_channel", "Updates the dump channel used for this bot in the current server!", false)
                .add_option(cmd_opt(dpp::co_channel, "channel", "The channel dedicated to this bot.", false)
                    .add_channel_type(dpp::channel_type::CHANNEL_TEXT)))
            //Setup help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the setup group command", false)))
        //Insert command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "insert", "Insert a new record in the database", false)
            //Insert url command
            .add_option(cmd_opt(dpp::co_sub_command, "url", "Insert a new record from an url", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_string, "url", "Identifying url for the discord message/attachment", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_url()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_url())))
                .add_option(cmd_opt(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Insert file command
            .add_option(cmd_opt(dpp::co_sub_command, "file", "Insert a new record from a given file", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_attachment, "file", "The attachment to add to the record", true))
                .add_option(cmd_opt(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Insert text command
            .add_option(cmd_opt(dpp::co_sub_command, "text", "Insert a new record from a given text", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Insert help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the insert group command", false)))
        //Update command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "update", "Updates a database record", false)
            //Update description command
            .add_option(cmd_opt(dpp::co_sub_command, "description", "Updates a database record's description", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_string, "description", "Description for the record", false)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_description()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_description()))))
            //Update help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the update group command", false)))
        //Delete command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "delete", "Deletes a database record", false)
            //Delete guild command
            .add_option(cmd_opt(dpp::co_sub_command, "guild", "Deletes all records for the guild", false))
            //Delete self command
            .add_option(cmd_opt(dpp::co_sub_command, "self", "Deletes all records related to the user from the entire db (other servers included)", false))
            //Delete user command
            .add_option(cmd_opt(dpp::co_sub_command, "user", "Deletes all records from the given user for the guild", false)
                .add_option(cmd_opt(dpp::co_user, "user", "Select for deletion only the records created by this user", true)))
            //Delete single command
            .add_option(cmd_opt(dpp::co_sub_command, "single", "Deletes one record for the guild", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record to delete", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_user, "owner", "Fill this parameter in case you want to delete a record owned by someone else", false)))
            //Delete help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the delete command group", false)))
        //Select command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "select", "Selects and returns a single database record", false)
            //Select single command
            .add_option(cmd_opt(dpp::co_sub_command, "single", "Selects and returns one record", false)
                .add_option(cmd_opt(dpp::co_string, "name", "Identifying name for the record", true)
                    .set_min_length(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))
                    .set_max_length(static_cast<int64_t>(mln::constants::get_max_characters_text_id())))
                .add_option(cmd_opt(dpp::co_boolean, "broadcast", "Determines if the result of the operation will be seen by everyone or only by the user", false)))
            //Select help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the select command group", false)))
        //Show command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "show", "Selects and returns several records' info", false)
            //Show all command
            .add_option(cmd_opt(dpp::co_sub_command, "all", "Selects and returns all records' info", false))
            //Show user command
            .add_option(cmd_opt(dpp::co_sub_command, "user", "Selects and returns all user records' info", false)
                .add_option(cmd_opt(dpp::co_user, "user", "The database will show all the records created by this user", true)))
            //Show help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the show command group", false)))
        //help command
        .add_option(cmd_opt(dpp::co_sub_command_group, "help", "Gives generic information about the db group command", false)
            .add_option(cmd_opt(dpp::co_sub_command, "generic", "Gives generic information about the db group command", false)))
        .add_option(cmd_opt(dpp::co_sub_command_group, "privacy", "Gives information about the db group command privacy policy", false)
            .add_option(cmd_opt(dpp::co_sub_command, "policy", "Gives information about the db group command privacy policy", false)))
    )) {}
//TODO I need to add encryption for user data, other security measures and inform user of data stored
//TODO https://support-dev.discord.com/hc/en-us/articles/8562894815383-Discord-Developer-Terms-of-Service section 5, add /db privacy policy command to explain data usage/storage and deletion
//TODO https://gdpr.eu/
dpp::task<void> mln::db::command(const dpp::slashcommand_t& event_data){
    static const std::unique_ptr<mln::base_db_command> commands[]{
        std::make_unique<mln::db_insert>(mln::db_insert{delta()}),
        std::make_unique<mln::db_update_dump_channel>(mln::db_update_dump_channel{delta()}),
        std::make_unique<mln::db_select>(mln::db_select{delta()}),
        std::make_unique<mln::db_show>(mln::db_show{delta()}),
        std::make_unique<mln::db_delete>(mln::db_delete{delta()}),
        std::make_unique<mln::db_update>(mln::db_update{delta()}),
        std::make_unique<mln::db_help>(mln::db_help{delta()}),
        std::make_unique<mln::db_privacy>(mln::db_privacy{delta()}),
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_insert_sub_commands{
        {"url", {commands[0], mln::db_command_type::url}},
        {"file", {commands[0], mln::db_command_type::file}},
        {"text", {commands[0], mln::db_command_type::text}},
        {"help", {commands[0], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_update_sub_commands{
        {"description", {commands[5], mln::db_command_type::description}},
        {"help", {commands[5], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_delete_sub_commands{
        {"single", {commands[4], mln::db_command_type::single}},
        {"user", {commands[4], mln::db_command_type::user}},
        {"self", {commands[4], mln::db_command_type::self}},
        {"guild", {commands[4], mln::db_command_type::guild}},
        {"help", {commands[4], mln::db_command_type::help}},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_setup_sub_commands{
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
        {"setup", s_allowed_setup_sub_commands},
        {"select", s_allowed_select_sub_commands},
        {"show", s_allowed_show_sub_commands},
        {"delete", s_allowed_delete_sub_commands},
        {"update", s_allowed_update_sub_commands},
        {"help", s_allowed_help_sub_commands},
        {"privacy", s_allowed_privacy_sub_commands},
    };

    //Return error if event_data or cmd_data are incorrect
    dpp::command_interaction cmd_interaction = event_data.command.get_command_interaction();
    if (event_data.command.id == 0 || event_data.command.guild_id == 0 || event_data.command.channel_id == 0 || cmd_interaction.options.size() == 0) {
        event_data.reply(dpp::message{"Failed to proceed with db command, event data is incorrect!"}.set_flags(dpp::m_ephemeral));
        co_return;
    }

    //Get the mapper for the primary cmds, return error if not found
    dpp::command_data_option primary_cmd = cmd_interaction.options[0];
    const auto& it = s_allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == s_allowed_primary_sub_commands.end()) {
        event_data.reply(dpp::message{"Couldn't find primary sub_command " + primary_cmd.name}.set_flags(dpp::m_ephemeral));
        co_return;
    }

    //Return error if primary_cmd is incorrect
    if (primary_cmd.options.size() == 0) {
        event_data.reply(dpp::message{"Failed to proceed with db command, event data is incorrect!"}.set_flags(dpp::m_ephemeral));
        co_return;
    }

    //Get the sub_command handler, return an error if not found
    const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>>& mapper = it->second;
    dpp::command_data_option sub_command = primary_cmd.options[0];
    const auto& sub_it = mapper.find(sub_command.name);
    if (sub_it == mapper.end()) {
        event_data.reply(dpp::message{"Couldn't find " + primary_cmd.name + "'s sub_command " + sub_command.name}.set_flags(dpp::m_ephemeral));
        co_return;
    }

    //Data to be given to the selected command function
    db_cmd_data_t cmd_data{nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0};
    std::optional<dpp::async<dpp::confirmation_callback_t>> thinking{std::nullopt};
    db_command_type cmd_type = std::get<1>(sub_it->second);

    db_init_type_flag init_requested = std::get<0>(sub_it->second)->get_requested_initialization_type(cmd_type);

    //If the command requests thinking, start thinking
    const bool is_thinking = (init_requested & db_init_type_flag::thinking) != db_init_type_flag::none;
    if (is_thinking) {
        thinking = event_data.co_thinking(true);
    }

    //If the command requests cmd_data, retrieve it
    if ((init_requested & db_init_type_flag::cmd_data) != db_init_type_flag::none) {

        //Prepare most common data for commands
        //Retrieve guild data
        std::optional<std::shared_ptr<const dpp::guild>> guild = mln::caches::get_guild(event_data.command.guild_id);
        if (!guild.has_value()) {
            guild = co_await mln::caches::get_guild_task(event_data.command.guild_id);
            if (!guild.has_value()) {
                //Error can't find guild
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve guild data! guild_id: "
                    + std::to_string(event_data.command.guild_id));
                co_return;
            }
        }
        cmd_data.cmd_guild = guild.value();

        //Retrieve channel data
        std::optional<std::shared_ptr<const dpp::channel>> channel = mln::caches::get_channel(event_data.command.channel_id, &event_data);
        if (!channel.has_value()) {
            channel = co_await mln::caches::get_channel_task(event_data.command.channel_id);
            if (!channel.has_value()) {
                //Error can't find channel
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve channel data! channel_id: "
                    + std::to_string(event_data.command.channel_id));
                co_return;
            }
        }
        cmd_data.cmd_channel = channel.value();

        //Retrieve command user and bot information
        std::optional<std::shared_ptr<const dpp::guild_member>> user = mln::caches::get_member({cmd_data.cmd_guild->id, event_data.command.usr.id}, &event_data);
        if (!user.has_value()) {
            user = co_await mln::caches::get_member_task({cmd_data.cmd_guild->id, event_data.command.usr.id});
            if (!user.has_value()) {
                //Error can't find user
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve command user data! user_id: "
                    + std::to_string(event_data.command.usr.id));
                co_return;
            }
        }
        cmd_data.cmd_usr = user.value();

        std::optional<std::shared_ptr<const dpp::guild_member>> bot = mln::caches::get_member({cmd_data.cmd_guild->id, event_data.command.application_id}, &event_data);
        if (!bot.has_value()) {
            bot = co_await mln::caches::get_member_task({cmd_data.cmd_guild->id, event_data.command.application_id});
            if (!bot.has_value()) {
                //Error can't find bot
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve command bot data! bot id: "
                    + std::to_string(event_data.command.application_id));
                co_return;
            }
        }
        cmd_data.cmd_bot = bot.value();

        //Retrieve user and bot perms, then return an error if the user and the bot don't have the required permissions
        std::optional<dpp::permission> user_perm = mln::perms::get_computed_permission(*(cmd_data.cmd_guild), *(cmd_data.cmd_channel), *(cmd_data.cmd_usr), &event_data);
        if (!user_perm.has_value()) {
            user_perm = co_await mln::perms::get_computed_permission_task(*(cmd_data.cmd_guild), *(cmd_data.cmd_channel), *(cmd_data.cmd_usr), &event_data);
            if (!user_perm.has_value()) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve user permission data! user_id: "
                    + std::to_string(cmd_data.cmd_usr->user_id));
                co_return;
            }
        }
        cmd_data.cmd_usr_perm = user_perm.value();

        std::optional<dpp::permission> bot_perm = mln::perms::get_computed_permission(*(cmd_data.cmd_guild), *(cmd_data.cmd_channel), *(cmd_data.cmd_bot), &event_data);
        if (!bot_perm.has_value()) {
            bot_perm = co_await mln::perms::get_computed_permission_task(*(cmd_data.cmd_guild), *(cmd_data.cmd_channel), *(cmd_data.cmd_bot), &event_data);
            if (!bot_perm.has_value()) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve bot permission data! bot id: "
                    + std::to_string(cmd_data.cmd_bot->user_id));
                co_return;
            }
        }
        cmd_data.cmd_bot_perm = bot_perm.value();
    }

    //If requested, determine the dump channel
    if ((init_requested & mln::db_init_type_flag::dump_channel) != db_init_type_flag::none) {

        //Recover dump channel id from cache if possible. Default value for dump channel is the channel the command was invoked in
        uint64_t dump_channel_id{event_data.command.channel_id};
        const std::optional<uint64_t> opt_channel = mln::caches::get_dump_channel_id(event_data.command.guild_id);
        if (!opt_channel.has_value()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to retrieve dump channel, either an internal error occurred or the guild from which this command was called is not registered in the database! Contact the bot developer.");
            co_return;
        }

        if (opt_channel.value() != 0) {
            dump_channel_id = opt_channel.value();
        }

        //Get the full channel data from cache if possible
        if (dump_channel_id == event_data.command.channel_id && cmd_data.cmd_channel) {
            cmd_data.dump_channel = cmd_data.cmd_channel;
        } else {
            std::optional<std::shared_ptr<const dpp::channel>> dump_channel_opt = mln::caches::get_channel(dump_channel_id, &event_data);
            if (!dump_channel_opt.has_value()) {
                dump_channel_opt = co_await mln::caches::get_channel_task(dump_channel_id);
                if (!dump_channel_opt.has_value()) {
                    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve dump channel data! dump channel id: "
                        + std::to_string(dump_channel_id));
                    co_return;
                }
            }
            cmd_data.dump_channel = dump_channel_opt.value();
        }

        //If the cmd bot and cmd guild data has been filled, get the dump channel bot perms
        if (cmd_data.cmd_bot && cmd_data.cmd_guild) {
            std::optional<dpp::permission> bot_dump_channel_perm = mln::perms::get_computed_permission(*(cmd_data.cmd_guild), *(cmd_data.dump_channel), *(cmd_data.cmd_bot), &event_data);
            if (!bot_dump_channel_perm.has_value()) {
                bot_dump_channel_perm = co_await mln::perms::get_computed_permission_task(*(cmd_data.cmd_guild), *(cmd_data.dump_channel), *(cmd_data.cmd_bot), &event_data);
                if (!bot_dump_channel_perm.has_value()) {
                    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve bot permission data! bot id: "
                        + std::to_string(cmd_data.cmd_bot->user_id));
                    co_return;
                }
            }
            cmd_data.dump_channel_bot_perm = bot_dump_channel_perm.value();
        }
    }

    co_await std::get<0>(sub_it->second)->command(event_data, cmd_data, cmd_type, thinking);
}