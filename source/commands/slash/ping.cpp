#include "commands/slash/ping.h"
#include "utility/utility.h"
#include "utility/response.h"

#include <dpp/cluster.h>

#include <format>

mln::ping::ping(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("ping", "Ping pong!", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {}

dpp::task<void> mln::ping::command(const dpp::slashcommand_t& event_data) const {   
    const double websocket_ping = event_data.from ? event_data.from->websocket_ping : 0.0;

    if (mln::utility::conf_callback_is_error(
        co_await mln::response::make_response(true, event_data, std::format("Websocket ping = {} seconds.\nRest ping = {} seconds.", websocket_ping, bot().rest_ping)), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the ping text!");
    }
}