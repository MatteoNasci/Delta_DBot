#include "commands/slash/db/db.h"
#include "bot_delta.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "commands/slash/db/db_update_dump_channel.h"
#include "commands/slash/db/db_insert.h"
#include "commands/slash/db/db_select.h"
#include "commands/slash/db/db_show.h"

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
            //Update url command
            .add_option(cmd_opt(dpp::co_sub_command, "url", "Updates a database record from an url", false))
            //Update file command
            .add_option(cmd_opt(dpp::co_sub_command, "file", "Updates a database record from a given file", false))
            //Update text command
            .add_option(cmd_opt(dpp::co_sub_command, "text", "Updates a database record from a given text", false))
            //Update description command
            .add_option(cmd_opt(dpp::co_sub_command, "description", "Updates a database record's description", false))
            //Update help command
            .add_option(cmd_opt(dpp::co_sub_command, "help", "Gives detailed information about the update group command", false)))
        //Delete command group
        .add_option(cmd_opt(dpp::co_sub_command_group, "delete", "Deletes a database record", false)
            //Delete all command
            .add_option(cmd_opt(dpp::co_sub_command, "all", "Deletes all records for the guild", false))
            //Delete user command
            .add_option(cmd_opt(dpp::co_sub_command, "user", "Deletes all records from the given user for the guild", false))
            //Delete single command
            .add_option(cmd_opt(dpp::co_sub_command, "single", "Deletes one record for the guild", false))
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
        //TODO add ordering to the results, also test if with co_user you can insert the raw user id
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
    )) {}
//TODO add bot logging to ALL co_ function failures, remove all variants that aren't co_ everywhere (similar to insert_url)
dpp::task<void> mln::db::command(const dpp::slashcommand_t& event_data){
    static const std::unique_ptr<mln::base_db_command> commands[]{
        std::make_unique<mln::db_insert>(mln::db_insert{delta()}),
        std::make_unique<mln::db_update_dump_channel>(mln::db_update_dump_channel{delta()}),
        std::make_unique<mln::db_select>(mln::db_select{delta()}),
        std::make_unique<mln::db_show>(mln::db_show{delta()}),
    };
    const std::unique_ptr<mln::base_db_command>& x = commands[0];
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_insert_sub_commands{
        {"url", {commands[0], mln::db_command_type::url}},
        {"file", {commands[0], mln::db_command_type::file}},
        {"text", {commands[0], mln::db_command_type::text}},
        {"help", {commands[0], mln::db_command_type::help}},

        //{"insert", std::make_shared<mln::db_insert>(delta())},
        //{"insert_update", std::make_shared<mln::db_insert_update>(delta())},
        //{"update", std::make_shared<mln::db_update>(delta())},
        //{"insert_url", std::make_shared<mln::db_insert>(delta())},
        //{"insert_update_url", std::make_shared<mln::db_insert_update_url>(delta())},
        //{"update_url", std::make_shared<mln::db_update_url>(delta())},
    };
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_update_sub_commands{
        //{"insert", s_allowed_file_sub_commands.at("insert")},
        //{"insert_update", s_allowed_file_sub_commands.at("insert_update")},
        //{"update", s_allowed_file_sub_commands.at("update")},
        //{"insert_url", s_allowed_file_sub_commands.at("insert_url")},
        //{"insert_update_url", s_allowed_file_sub_commands.at("insert_update_url")},
        //{"update_url", s_allowed_file_sub_commands.at("update_url")},
        //TODO use caching to store last retrieved msgs (to reduce API calls), make a ascending priority queue where each msgs gets associated with an ascending number. When the occupied storage space gets too high I free the message/s with the lowest value (min heap)
        //TODO PRIORITY: I should change how I store the urls. As of now text and file use different type of url (file the url for attachment, text the url for msgs). There is no need for this inconsistency, just make both file and txtx use the msg url that links to the stored message, then let file handle the retrieval of files and text the retrieval of text stuff.
        //TODO add version of the text insert, insert_update and update to take a msg url instead, with optional bool param to delete original msg linked to the given url.
    };//TODO also make sure to delete stuff in the dump channel when updating/deleting stuff, if the original stuff is no longer usefull i might as well delete it
    static const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>> s_allowed_delete_sub_commands{
        //{"update_description", std::make_shared<mln::db_update_description>(delta())},
        //{"delete", std::make_shared<mln::db_delete>(delta())},
        //{"select", std::make_shared<mln::db_select>(delta())},
        //{"show_records", std::make_shared<mln::db_show_records>(delta())},
        //{"change_type", std::make_shared<mln::db_change_type>(delta())},
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
    static const std::unordered_map<std::string, const std::unordered_map<std::string, std::tuple<const std::unique_ptr<mln::base_db_command>&, db_command_type>>&> s_allowed_primary_sub_commands{
        {"insert", s_allowed_insert_sub_commands},
        {"setup", s_allowed_setup_sub_commands},
        {"select", s_allowed_select_sub_commands},
        {"show", s_allowed_show_sub_commands},
        //{"delete", s_allowed_delete_sub_commands},
    };

    //Return error if event_data or cmd_data are incorrect
    dpp::command_interaction cmd_interaction = event_data.command.get_command_interaction();
    if (event_data.command.id == 0 || cmd_interaction.options.size() == 0) {
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
    db_cmd_data_t cmd_data{nullptr, nullptr, nullptr, nullptr, nullptr};
    std::optional<dpp::async<dpp::confirmation_callback_t>> thinking{std::nullopt};
    db_command_type cmd_type = std::get<1>(sub_it->second);

    db_init_type_flag init_requested = std::get<0>(sub_it->second)->get_requested_initialization_type(cmd_type);

    //If the command requires for thinking, start thinking request
    if ((init_requested & db_init_type_flag::thinking) != db_init_type_flag::none) {
        thinking = event_data.co_thinking(true);
    }
    //If the command doesn't request for additional data just execute it
    if ((init_requested & db_init_type_flag::cmd_data) == db_init_type_flag::none &&
        (init_requested & db_init_type_flag::dump_channel) == db_init_type_flag::none) {
        co_await std::get<0>(sub_it->second)->command(event_data, cmd_data, cmd_type, thinking);
        co_return;
    }

    //Prepare most common data for commands
    //Retrieve guild data
    std::tuple<dpp::guild*, dpp::guild> guild_pair = co_await mln::utility::get_guild(event_data, delta()->bot);
    cmd_data.cmd_guild = std::get<0>(guild_pair);
    if (cmd_data.cmd_guild == nullptr) {
        //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
        cmd_data.cmd_guild = &std::get<1>(guild_pair);
    }

    //Retrieve channel data
    std::tuple<dpp::channel*, dpp::channel> channel_pair = co_await mln::utility::get_channel(event_data, event_data.command.channel_id, delta()->bot);
    cmd_data.cmd_channel = std::get<0>(channel_pair);
    if (cmd_data.cmd_channel == nullptr) {
        //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
        cmd_data.cmd_channel = &std::get<1>(channel_pair);
    }

    //If we failed to find the guild the command originated from or the channel, we return an error
    if (cmd_data.cmd_channel->id == 0 || cmd_data.cmd_guild->id == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve guild and channel data!");
        co_return;
    }

    //Retrieve command user and bot information
    const std::optional<dpp::guild_member> user = co_await mln::utility::get_member(event_data, cmd_data.cmd_guild, event_data.command.usr.id, delta()->bot);
    const std::optional<dpp::guild_member> bot = co_await mln::utility::get_member(event_data, cmd_data.cmd_guild, event_data.command.application_id, delta()->bot);

    //If we failed to find the user who issued the command or the bot, we return an error
    if (!user.has_value() || user->user_id == 0 || !bot.has_value() || bot->user_id == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve bot and user data!");
        co_return;
    }

    cmd_data.cmd_usr = &user.value();
    cmd_data.cmd_bot = &bot.value();

    //Return an error if the user and the bot don't have the required permissions
    const bool can_use_slashcommand = mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.cmd_channel, {user, bot}, dpp::permissions::p_use_application_commands);
    if (!can_use_slashcommand) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, either the user or the bot do not have the slashcommand required permission!", {true, dpp::loglevel::ll_debug});
        co_return;
    }

    //If requested, determine the dump channel
    if ((init_requested & mln::db_init_type_flag::dump_channel) != db_init_type_flag::none) {

        //Recover dump channel from cache if possible. Default value for dump channel is the channel the command was invoked in
        uint64_t dump_channel_id{cmd_data.cmd_channel->id};
        const std::optional<uint64_t> opt_channel = delta()->dump_channels_cache.get_element(cmd_data.cmd_guild->id);
        if (opt_channel.has_value()) {
            dump_channel_id = opt_channel.value();
        } else {
            //If no cache dump channel found, recover dump channel from database, if not present (the stored value is '0') leave the local channel as the dump channel. If no value at all found then either an error occurred or the given guild is not registered in the database, return an error
            const std::optional<uint64_t> opt_db_dump_channel = delta()->get_db_dump_channel(cmd_data.cmd_guild->id);
            if (opt_db_dump_channel.has_value()) {
                if (opt_db_dump_channel.value() != 0) {
                    dump_channel_id = opt_db_dump_channel.value();
                }
            } else {
                //No record found for the given guild, return an error
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed to retrieve dump channel, either an internal error occurred or the guild from which this command was called is not registered in the database! Contact the bot developer.");
                co_return;
            }
        }

        //If the dump channel is the same as the channel the command came from there's no need to get_channel, just use the cmd_channel. Return error if no dump channel found
        cmd_data.dump_channel = cmd_data.cmd_channel;
        std::tuple<dpp::channel*, dpp::channel> ch_pair;
        if (cmd_data.cmd_channel->id != dump_channel_id) {
            ch_pair = co_await mln::utility::get_channel(event_data, dump_channel_id, delta()->bot);

            cmd_data.dump_channel = std::get<0>(ch_pair);
            if (cmd_data.dump_channel == nullptr) {
                //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
                cmd_data.dump_channel = &std::get<1>(ch_pair);
            }
        }

        if (cmd_data.dump_channel->id == 0) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed command, failed to retrieve dump channel data!");
            co_return;
        }
    }

    co_await std::get<0>(sub_it->second)->command(event_data, cmd_data, cmd_type, thinking);
}