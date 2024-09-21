#include "events/guild_create_runner.h"
#include "commands/guild/create/insert_guild_db.h"

#include <dpp/dispatcher.h>
#include <dpp/cluster.h>

void mln::guild_create_runner::attach_event() {
    actions.emplace_back(std::make_unique<mln::insert_guild_db>(bot(), database()));

    bot().on_guild_create([this](const dpp::guild_create_t& event) ->dpp::task<void> {
        for (const std::unique_ptr<mln::base_guild_create>& action : this->actions) {
            co_await action->command(event);
        }
    });
}

mln::guild_create_runner::guild_create_runner(dpp::cluster& cluster, database_handler& db) : base_event{ cluster, db }
{
}
