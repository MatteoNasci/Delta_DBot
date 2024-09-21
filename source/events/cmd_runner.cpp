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

mln::cmd_runner::cmd_runner(dpp::cluster& cluster, database_handler& db) : base_event{ cluster, db }, event_id{}, initialized{ false }, id_to_cmd_map{}
{
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

    ptr = std::make_unique<mln::help>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::report>(bot(), database());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    ptr = std::make_unique<mln::changelog>(bot());
    actions.emplace(ptr->get_cmd().name, std::move(ptr));

    if (initialized) {
        bot().on_slashcommand.detach(event_id);
        id_to_cmd_map.clear();
    }
    event_id = bot().on_slashcommand([this](const dpp::slashcommand_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event);
        }
        co_return;
    });

    initialized = true;
}

void mln::cmd_runner::add_command_ids(const dpp::slashcommand_map& map)
{
    //TODO fix this implementation, commands are not seen
    return;

    if (actions.size() == 0) {
        return;
    }

    std::unordered_set<std::unique_ptr<mln::base_slashcommand>*> unique_checker{};
    for (const std::pair<dpp::snowflake, dpp::slashcommand>& pair : map) {
        if (pair.second.type != dpp::slashcommand_contextmenu_type::ctxm_chat_input) {
            continue;
        }

        const std::string key = pair.second.name;
        const auto& it = actions.find(key);
        if (it == actions.end()) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to add command ids for cmd runner, name [{}] not found in actions map! Reverting to actions usage...", key));
            return;
        }

        if (unique_checker.contains(&(it->second))) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to add command ids for cmd runner, name [{}] found multiple times in actions! Reverting to actions usage...", key));
            return;
        }

        unique_checker.insert(&(it->second));
    }

    if (unique_checker.size() != actions.size()) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to add command ids for cmd runner. Actions size: [{}], ids size: [{}]. Reverting to actions usage...", actions.size(), unique_checker.size()));
        return;
    }

    bot().log(dpp::loglevel::ll_debug, "Changing cmd runner from string to id map...");

    id_to_cmd_map.clear();
    id_to_cmd_map.reserve(actions.size());
    
    for (const std::pair<dpp::snowflake, dpp::slashcommand>& pair : map) {
        if (pair.second.type != dpp::slashcommand_contextmenu_type::ctxm_chat_input) {
            continue;
        }

        const std::string key = pair.second.name;
        std::unordered_map<std::string, std::unique_ptr<base_slashcommand>>::iterator it = actions.find(key);
        if (it != actions.end()) {
            id_to_cmd_map.insert(std::make_pair(pair.first, std::move(it->second)));
        }
    }
    
    if (initialized) {
        bot().on_slashcommand.detach(event_id);
        actions.clear();
    }

    event_id = bot().on_slashcommand([this](const dpp::slashcommand_t& event) -> dpp::task<void> {
        const size_t key = event.command.id;
        if (const auto& function_it = id_to_cmd_map.find(key); function_it != id_to_cmd_map.end()) {
            co_await function_it->second->command(event);
        }
        co_return;
        });

    initialized = true;
}
