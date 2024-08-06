#include "commands/ping.h"

dpp::task<void> mln::ping::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
    event.reply(dpp::message("Pong! Ping " + std::to_string(event.from->websocket_ping) + " seconds.").set_flags(dpp::m_ephemeral));
    co_return;
}
dpp::slashcommand mln::ping::get_command(dpp::cluster& bot){
    return dpp::slashcommand(mln::ping::get_command_name(), "Ping pong!", bot.me.id);
}
std::string mln::ping::get_command_name(){
    return "ping";
}