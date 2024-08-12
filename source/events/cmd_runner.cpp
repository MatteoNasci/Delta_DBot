#include "events/cmd_runner.h"
#include "general/commands.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/job.h>

void mln::cmd_runner::attach_event(bot_delta* const delta){
    std::unique_ptr<mln::base_slashcommand> ptr(std::make_unique<mln::ping>(mln::ping(delta)));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::pm>(mln::pm(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::add_emoji>(mln::add_emoji(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::add_role>(mln::add_role(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::avatar>(mln::avatar(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::bot_info>(mln::bot_info(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::db>(mln::db(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::help>(mln::help(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::msgs_get>(mln::msgs_get(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    ptr = std::make_unique<mln::report>(mln::report(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    delta->bot.on_slashcommand([this](const dpp::slashcommand_t& event) -> dpp::job {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            function_it->second->command(event);
        }
        co_return;
    });
}
