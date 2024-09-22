#include "events/cmd_runner.h"
#include "commands/slash/ping.h"
#include "commands/slash/pm.h"
#include "commands/slash/add_emoji.h"
#include "commands/slash/add_role.h"
#include "commands/slash/avatar.h"
#include "commands/slash/bot_info.h"
#include "commands/slash/db/db.h"
#include "commands/slash/help.h"
#include "commands/slash/report.h"
#include "commands/slash/changelog.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/job.h>
#include <dpp/cluster.h>

#include <memory>
#include <unordered_set>

mln::cmd_runner::cmd_runner(dpp::cluster& cluster, database_handler& db) : base_event{ cluster, db }, event_id{}, initialized{ false }
{
}
#include "utility/utility.h"
#include <chrono>
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

    event_id = bot().on_slashcommand([this](const dpp::slashcommand_t& event_data) -> dpp::task<void> {
        const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        const std::string key = event_data.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event_data);
        }
        const std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        mln::utility::create_event_log_error(event_data, bot(), std::format("Event over, elapsed time: [{}].", std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch() - start.time_since_epoch())));
        co_return;
    });

    initialized = true;
}
