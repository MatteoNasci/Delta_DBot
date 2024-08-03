#include "commands/thinking.h"

void thinking::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    event.thinking(true, [event](const dpp::confirmation_callback_t& callback) {
                    event.edit_original_response(dpp::message("thonk"));
                });
}
dpp::slashcommand thinking::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(thinking::get_command_name(), "Thinking example...", bot.me.id);
}
std::string thinking::get_command_name(){
    return "thinking";
}