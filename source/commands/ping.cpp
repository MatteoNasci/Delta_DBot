#include "commands/ping.h"

dpp::task<void> ping::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
    event.reply(dpp::message("Pong! Ping " + std::to_string(event.from->websocket_ping)).set_flags(dpp::m_ephemeral));
    co_return;
}
dpp::slashcommand ping::get_command(dpp::cluster& bot){
    return dpp::slashcommand(ping::get_command_name(), "Ping pong!", bot.me.id);
}
std::string ping::get_command_name(){
    return "ping";
}