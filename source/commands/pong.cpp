#include "commands/pong.h"

dpp::task<void> pong::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    event.reply("Ping!");
    co_return;
}
dpp::slashcommand pong::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(pong::get_command_name(), "Ping pong!", bot.me.id);
}
std::string pong::get_command_name(){
    return "pong";
}