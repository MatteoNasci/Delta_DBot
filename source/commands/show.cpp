#include "commands/show.h"
#include <variant>

dpp::task<void> mln::show::command(mln::bot_delta_data_t& data, const dpp::slashcommand_t& event){
    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    /* Get the file id from the parameter attachment. */
    dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
    /* Get the attachment that the user inputted from the file id. */
    dpp::attachment att = event.command.get_resolved_attachment(file_id);
    /* Reply with the file as a URL. */
    dpp::message msg(att.url);
    if (!broadcast) {
        msg.set_flags(dpp::m_ephemeral);
    }
    event.reply(msg);
    co_return;
}
dpp::slashcommand mln::show::get_command(dpp::cluster& bot){
    return dpp::slashcommand(mln::show::get_command_name(), "Show an uploaded file", bot.me.id)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel"));
}
std::string mln::show::get_command_name(){
    return "show";
}