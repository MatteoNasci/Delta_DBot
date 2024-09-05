#include "commands/slash/ping.h"
#include "bot_delta.h"

mln::ping::ping(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("ping", "Ping pong!", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands))) {}

dpp::task<void> mln::ping::command(const dpp::slashcommand_t& event_data){
    if (event_data.from == nullptr) {
        event_data.reply(dpp::message{"Failed to retrieve client attached to user!"}.set_flags(dpp::m_ephemeral));
        co_return;
    }

    event_data.reply(dpp::message{"Pong! Ping = " + std::to_string(event_data.from->websocket_ping) + " seconds."}.set_flags(dpp::m_ephemeral));
}