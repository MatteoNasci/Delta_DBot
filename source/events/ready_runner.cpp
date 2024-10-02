#include "commands/ready/base_ready.h"
#include "commands/ready/register_commands.h"
#include "events/base_event.h"
#include "events/ready_runner.h"
#include "threads/jobs_runner.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

void mln::ready_runner::attach_event(){
    actions.clear();

    actions.emplace_back(std::make_unique<mln::register_commands>(bot(), runner, ctx_runner));

    if (initialized) {
        bot().on_ready.detach(event_id);
        initialized = false;
    }

    if (actions.size() == 0) {
        return;
    }

    event_id = bot().on_ready([this](const dpp::ready_t& event_data) -> dpp::task<void> {
        for (const std::unique_ptr<mln::base_ready>& action : this->actions) {
            if (action->use_job()) {
                const std::optional<std::function<void()>> command_job = action->job(event_data);
                if (command_job.has_value()) {
                    jobs_handler().add_job(command_job.value());
                }
            }
            else {
                co_await action->command(event_data);
            }
        }
    });

    initialized = true;
}

mln::ready_runner::ready_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner, cmd_runner& in_runner, cmd_ctx_runner& in_ctx_runner) : base_event{ cluster, db, j_runner }, event_id{}, initialized{ false }, runner{ in_runner }, ctx_runner{ in_ctx_runner }
{
}
