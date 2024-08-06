#include "commands/db.h"
#include "sqlite3.h"

#include <variant>
#include <unordered_set>

dpp::task<void> mln::db::command(mln::bot_delta_data_t& data, const dpp::slashcommand_t& event){
    const bool broadcast = std::get<bool>(event.get_parameter("broadcast"));
    const dpp::command_interaction cmd_data = event.command.get_command_interaction();
    
    static const std::unordered_set<std::string> allowed_sub_commands{
        {"insert"},
        {"select"},
        {"show records"},
        {"update"},
        {"remove"}
    };

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
    co_return;
}
dpp::slashcommand mln::db::get_command(dpp::cluster& bot){
    return dpp::slashcommand(mln::db::get_command_name(), "Manage the database.", bot.me.id)
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel.", true))
        .add_option(dpp::command_option(dpp::co_sub_command, "insert", "Inserts a new record in the db. It will fail if the given name is not unique!", false)
            .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
            .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true)))
        .add_option(dpp::command_option(dpp::co_sub_command, "select", "Selects a record from the db. It will fail if the given name is not present!", false)
            .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true)))
        .add_option(dpp::command_option(dpp::co_sub_command, "show records", "Shows all the db records names.", false))
        .add_option(dpp::command_option(dpp::co_sub_command, "update", "Updates an existing record in the db. It will fail if the given name is not present!", false)
            .add_option(dpp::command_option(dpp::co_attachment, "file", "File to insert.", true))
            .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true)))
        .add_option(dpp::command_option(dpp::co_sub_command, "remove", "Removes an existing record in the db. It will fail if the given name is not present!", false)
            .add_option(dpp::command_option(dpp::co_attachment, "name", "Unique name to associate with the file.", true)));
}
std::string mln::db::get_command_name(){
    return "db";
}