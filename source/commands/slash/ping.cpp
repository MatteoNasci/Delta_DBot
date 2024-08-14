#include "commands/slash/ping.h"
#include "bot_delta.h"

mln::ping::ping(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("ping", "Ping pong!", delta->bot.me.id))) {}

dpp::job mln::ping::command(dpp::slashcommand_t event){
    event.reply(dpp::message("Pong! Ping = " + std::to_string(event.from->websocket_ping) + " seconds.").set_flags(dpp::m_ephemeral));
    co_return;
}