#include "commands/slash/add_emoji.h"
#include "commands/slash/add_role.h"
#include "commands/slash/avatar.h"
#include "commands/slash/base_slashcommand.h"
#include "commands/slash/bot_info.h"
#include "commands/slash/changelog.h"
#include "commands/slash/db/db.h"
#include "commands/slash/help.h"
#include "commands/slash/mog/mog.h"
#include "commands/slash/ping.h"
#include "commands/slash/pm.h"
#include "commands/slash/report.h"
#include "events/base_event.h"
#include "events/cmd_runner.h"
#include "threads/jobs_runner.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

mln::cmd_runner::cmd_runner(dpp::cluster& cluster, database_handler& db, jobs_runner& j_runner) : base_event{ cluster, db, j_runner }, event_id{}, initialized{ false }
{
}

mln::cmd_runner::~cmd_runner()
{
    if (initialized) {
        bot().on_slashcommand.detach(event_id);
        initialized = false;
    }
}

//NOTE: the on_event from dpp will await on the result (if the return type is a task<void>), which means the input event_data is guaranteed to live as long as I use only task or (maybe) coroutines as my commands return values (void or any non coro values are also fine). Using job might void the guarantee since it will not be co_awaited neither by me nor by dpp
void mln::cmd_runner::attach_event(){
    
    actions.clear();

    std::unique_ptr<mln::base_slashcommand> ptr{ std::make_unique<mln::ping>(bot()) };
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::pm>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::add_emoji>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::add_role>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::avatar>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::bot_info>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::db>(bot(), database());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::mog::mog>(bot(), database());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::help>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::report>(bot(), database());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::changelog>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    if (initialized) {
        bot().on_slashcommand.detach(event_id);
        initialized = false;
    }

    if (actions.size() == 0) {
        return;
    }

    event_id = bot().on_slashcommand([this](const dpp::slashcommand_t& event_data) -> void {
        const std::string key = event_data.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            if (function_it->second->use_job()) {
                const std::optional<std::function<void()>> command_job = function_it->second->job(event_data);
                if (command_job.has_value()) {
                    jobs_handler().add_job(command_job.value());
                }
            }
            else {
                function_it->second->command(event_data);
            }
        }
        return;
    });

    initialized = true;
}
