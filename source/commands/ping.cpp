#include "commands/ping.h"
#include "bot_delta.h"

dpp::task<void> mln::ping::command(const dpp::slashcommand_t& event){
    event.reply(dpp::message("Pong! Ping " + std::to_string(event.from->websocket_ping) + " seconds.").set_flags(dpp::m_ephemeral));
    co_return;
}
dpp::slashcommand mln::ping::get_command(){
    return dpp::slashcommand(mln::ping::get_command_name(), "Ping pong!", mln::bot_delta::delta().bot.me.id);
}
std::string mln::ping::get_command_name(){
    return "ping";
}