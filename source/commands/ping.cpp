#include "commands/ping.h"

void ping::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    event.reply("Pong!");
}
dpp::slashcommand ping::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(ping::get_command_name(), "Ping pong!", bot.me.id);
}
std::string ping::get_command_name(){
    return "ping";
}