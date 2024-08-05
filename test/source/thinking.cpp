#include "commands/thinking.h"

dpp::task<void> thinking::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    co_await event.co_thinking(true);
    event.edit_original_response(dpp::message("thonk"));
}
dpp::slashcommand thinking::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(thinking::get_command_name(), "Thinking example...", bot.me.id);
}
std::string thinking::get_command_name(){
    return "thinking";
}