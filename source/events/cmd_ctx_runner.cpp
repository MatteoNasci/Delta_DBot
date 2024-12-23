#include "commands/base_action.h"
#include "database/database_handler.h"
#include "events/base_event.h"
#include "events/cmd_ctx_runner.h"
#include "threads/jobs_runner.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>

mln::cmd_ctx_runner::cmd_ctx_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner) : base_event{ cluster, db, j_runner }, event_id{}, initialized{ false }
{
    cbot().log(dpp::loglevel::ll_debug, std::format("cmd_ctx_runner: [{}].", true));
}

mln::cmd_ctx_runner::~cmd_ctx_runner()
{
    if (initialized) {
        bot().on_user_context_menu.detach(event_id);
        initialized = false;
    }
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

    event_id = bot().on_user_context_menu([this](const dpp::user_context_menu_t& event_data) -> dpp::task<void> {
        const std::string key = event_data.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            if (function_it->second->use_job()) {
                const std::optional<std::function<void()>> func = function_it->second->job(event_data);
                if (func.has_value()) {
                    if (!jobs_handler().add_job(func.value())) {
                        bot().log(dpp::loglevel::ll_error, std::format("Failed to add job to jobs runner! cmd_ctx_runner, [{}].", key));
                    }
                }
            }
            else {
                function_it->second->command(event_data);
            }
        }
    });

    initialized = true;
}
