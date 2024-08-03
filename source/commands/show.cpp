#include "commands/show.h"
#include <variant>

void show::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Get the file id from the parameter attachment. */
    dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
    /* Get the attachment that the user inputted from the file id. */
    dpp::attachment att = event.command.get_resolved_attachment(file_id);
    /* Reply with the file as a URL. */
    event.reply(att.url);
}
dpp::slashcommand show::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(show::get_command_name(), "Show an uploaded file", bot.me.id)
                .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image"));
}
std::string show::get_command_name(){
    return "show";
}