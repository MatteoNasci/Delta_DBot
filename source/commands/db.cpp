#include "commands/db.h"

#include <variant>
#include <unordered_map>

enum class db_op : char {
    db_none = 0,
    db_insert = 1,
    db_select = 2,
    db_show_records_keys = 3,
    db_update = 4,
    db_remove = 5,
    db_set_db_channel = 6,
    db_force_db_save = 7
};

dpp::task<void> mln::db::command(mln::bot_delta_data_t& data, const dpp::slashcommand_t& event){
    const bool broadcast = std::get<bool>(event.get_parameter("broadcast"));
    const dpp::command_interaction cmd_data = event.command.get_command_interaction();

    static const std::unordered_map<std::string, db_op> allowed_sub_commands{
        {"insert", db_op::db_insert},
        {"select", db_op::db_select},
        {"show_records_keys", db_op::db_show_records_keys},
        {"update", db_op::db_update},
        {"remove", db_op::db_remove},
        {"set_db_channel", db_op::db_set_db_channel},
        {"force_db_save", db_op::db_force_db_save},
        {"", db_op::db_none},
    };
    //TODO this file will manage the db related to each guild, while a separate, light-weight db will be stored in delta_bot and holds minimal critical information (like which channel is used for this db for each guild). The former db will be stored in ram, the latter in actual storage
    auto waiting_res = event.co_thinking(!broadcast);
    bool thinking = true;
    for (const auto& sub_cmd : cmd_data.options) {
        if (const auto& it = allowed_sub_commands.find(sub_cmd.name); it != allowed_sub_commands.end()) {
            
        }else {

        }
    }

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
    co_await waiting_res;
    event.edit_response("Work in progress!");
    co_return;
}
dpp::slashcommand mln::db::get_command(dpp::cluster& bot){
    return dpp::slashcommand(mln::db::get_command_name(), "Manage the database.", bot.me.id)
        .add_option(dpp::command_option(dpp::co_sub_command_group, "op", "Perform an operation on the db", false)
            .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "select", "Selects a record from the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "show_records_keys", "Shows all the db records names.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "force_db_save", "Forces the db to save in the designated channel, instead of waiting for a timer.", false)
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
                .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "remove", "Removes an existing record in the db. It will fail if the given name is not present!", false)
                .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false)))
            .add_option(dpp::command_option(dpp::co_sub_command, "set_db_channel", "Sets the channel where the bot will store the db. Required before any db cmd!", false)
                .add_option(dpp::command_option(dpp::co_channel, "channel", "Channel the bot will use to store the database.", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel. Default: false", false))));
}
std::string mln::db::get_command_name(){
    return "db";
}