#include "commands/slash/base_slashcommand.h"
#include "commands/slash/ping.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/permissions.h>

#include <format>
#include <functional>
#include <optional>
#include <type_traits>

mln::ping::ping(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("ping"), "Ping pong!", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {
    cbot().log(dpp::loglevel::ll_debug, std::format("ping: [{}].", true));
}

dpp::job mln::ping::command(dpp::slashcommand_t event_data) {
    
    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed to ping, the event is incorrect!");
        co_return;
    }

    const double websocket_ping = event_data.from ? event_data.from->websocket_ping : 0.0;

    co_await mln::response::co_respond(lite_data, std::format("Websocket ping = {} seconds.\nRest ping = {} seconds.", websocket_ping, bot().rest_ping), false, "Failed to reply with the ping text!");
}

std::optional<std::function<void()>> mln::ping::job(dpp::slashcommand_t)
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::ping::use_job() const noexcept
{
    return false;
}
