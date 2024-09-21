#include "events/cmd_ctx_runner.h"

#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>

#include <unordered_set>

mln::cmd_ctx_runner::cmd_ctx_runner(dpp::cluster& cluster, database_handler& db) : base_event{ cluster, db }, event_id{}, initialized{ false }, id_to_cmd_map{}
{
}

void mln::cmd_ctx_runner::attach_event(){
    actions.clear();

    //No commands at the moment

    if (actions.size() == 0) {
        return;
    }

    if (initialized) {
        bot().on_user_context_menu.detach(event_id);
        id_to_cmd_map.clear();
    }

    event_id = bot().on_user_context_menu([this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event);
        }
    });

    initialized = true;
}

void mln::cmd_ctx_runner::add_command_ids(const dpp::slashcommand_map& map)
{
    //TODO fix this implementation, commands are not seen
    return;

    if (actions.size() == 0) {
        return;
    }

    std::unordered_set<std::unique_ptr<mln::base_ctx_command>*> unique_checker{};
    for (const std::pair<dpp::snowflake, dpp::slashcommand>& pair : map) {
        if (pair.second.type != dpp::slashcommand_contextmenu_type::ctxm_user) {
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
        if (pair.second.type != dpp::slashcommand_contextmenu_type::ctxm_user) {
            continue;
        }

        const std::string key = pair.second.name;
        std::unordered_map<std::string, std::unique_ptr<mln::base_ctx_command>>::iterator it = actions.find(key);
        if (it != actions.end()) {
            id_to_cmd_map.insert(std::make_pair(pair.first, std::move(it->second)));
        }
    }

    if (initialized) {
        bot().on_user_context_menu.detach(event_id);
        actions.clear();
    }

    event_id = bot().on_user_context_menu([this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        const size_t key = event.command.id;
        if (const auto& function_it = id_to_cmd_map.find(key); function_it != id_to_cmd_map.end()) {
            co_await function_it->second->command(event);
        }
        co_return;
        });

    initialized = true;
}
