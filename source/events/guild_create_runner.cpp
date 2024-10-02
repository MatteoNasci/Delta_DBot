#include "commands/guild/create/base_guild_create.h"
#include "commands/guild/create/insert_guild_db.h"
#include "events/base_event.h"
#include "events/guild_create_runner.h"
#include "threads/jobs_runner.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

void mln::guild_create_runner::attach_event() {
    actions.clear();

    actions.emplace_back(std::make_unique<mln::insert_guild_db>(bot(), database()));

    if (initialized) {
        bot().on_guild_create.detach(event_id);
        initialized = false;
    }

    if (actions.size() == 0) {
        return;
    }

    event_id = bot().on_guild_create([this](const dpp::guild_create_t& event_data) -> dpp::task<void> {
        for (const std::unique_ptr<mln::base_guild_create>& action : this->actions) {
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

mln::guild_create_runner::guild_create_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner) : base_event{ cluster, db, j_runner }, event_id{}, initialized{ false }
{
}

mln::guild_create_runner::~guild_create_runner()
{
}
