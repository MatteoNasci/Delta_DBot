#include "events/cmd_ctx_runner.h"

#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>

#include <unordered_set>

mln::cmd_ctx_runner::cmd_ctx_runner(dpp::cluster& cluster, database_handler& db) : base_event{ cluster, db }, event_id{}, initialized{ false }
{
}

void mln::cmd_ctx_runner::attach_event(){
    actions.clear();

    //No commands at the moment

    if (initialized) {
        bot().on_user_context_menu.detach(event_id);
        initialized = false;
    }

    if (actions.size() == 0) {
        return;
    }

    event_id = bot().on_user_context_menu([this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event);
        }
    });

    initialized = true;
}
