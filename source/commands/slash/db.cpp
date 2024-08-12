#include "commands/slash/db.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::db::db(bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("db", "Manage the database.", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_sub_command_group, "op", "Perform an operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "insert_replace", "Inserts or replaces a record in the db.", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "select", "Selects a record from the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "show_records", "Shows all the db records names.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "remove", "Removes an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_string, "name", "Unique name to associate with the file.", true)
                    .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id())))
                    .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_text_id()))))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))))) {}

dpp::job mln::db::command(dpp::slashcommand_t event){

    typedef std::function<dpp::task<void>(dpp::command_data_option&, const dpp::slashcommand_t&)> op_callback_t;
    static const std::unordered_map<std::string, op_callback_t> allowed_op_sub_commands{
        {"insert", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
        {"select", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
        {"show_records_keys", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
        {"update", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
        {"remove", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
        {"", [](dpp::command_data_option&, const dpp::slashcommand_t&) -> dpp::task<void> { co_return; }},
    };
    static const std::unordered_map<std::string, op_callback_t> allowed_other_sub_commands{
        
    };

    static const std::unordered_map<std::string, const std::unordered_map<std::string, op_callback_t>*> allowed_primary_sub_commands{
        {"op", &allowed_op_sub_commands},
    };

    dpp::command_interaction cmd_data = event.command.get_command_interaction();
    dpp::command_data_option primary_cmd = cmd_data.options[0];
    const auto& it = allowed_primary_sub_commands.find(primary_cmd.name);
    if (it == allowed_primary_sub_commands.end()) {
        event.reply("Couldn't find primary sub_command " + primary_cmd.name);
        co_return;
    }

    const auto* const mapper = it->second;
    dpp::command_data_option sub_command = primary_cmd.options[0];
    const auto& sub_it = mapper->find(sub_command.name);
    if (sub_it == mapper->end()) {
        event.reply("Couldn't find " + primary_cmd.name + " sub_command " + sub_command.name);
        co_return;
    }

    sub_it->second(sub_command, event);

    /* Get the file id from the parameter attachment. */
    /*dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));*/
    /* Get the attachment that the user inputted from the file id. */
    /*dpp::attachment att = event.command.get_resolved_attachment(file_id); */
    /* Reply with the file as a URL. */
    /*dpp::message msg(att.url);
    if (!broadcast) {
        msg.set_flags(dpp::m_ephemeral);
    }
    event.reply(msg);*/
}