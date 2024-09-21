#include "events/ready_runner.h"
#include "commands/ready/register_commands.h"

#include <dpp/dispatcher.h>
#include <dpp/cluster.h>

void mln::ready_runner::attach_event(){
    //dpp::cluster::co_global_bulk_command_create, dpp::cluster::co_global_commands_get
    actions.emplace_back(std::make_unique<mln::register_commands>(bot(), runner, ctx_runner));

    bot().on_ready([this](const dpp::ready_t& event) ->dpp::task<void> {
        for (const std::unique_ptr<mln::base_ready>& action : this->actions) {
            co_await action->command(event);
        }
    });
}

mln::ready_runner::ready_runner(dpp::cluster& cluster, database_handler& db, cmd_runner& in_runner, cmd_ctx_runner& in_ctx_runner) : base_event{ cluster, db }, runner{ in_runner }, ctx_runner{ in_ctx_runner }
{
}
