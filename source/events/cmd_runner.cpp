#include "events/cmd_runner.h"
#include "general/commands.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/job.h>

//NOTE: the on_event from dpp will await on the result (if the return type is a task<void>), which means the input event_data is guaranteed to live as long as I use only task or (maybe) coroutines as my commands return values (void or any non coro values are also fine). Using job might void the guarantee since it will not be co_awaited neither by me nor by dpp
void mln::cmd_runner::attach_event(mln::bot_delta* const delta){
    std::unique_ptr<mln::base_slashcommand> ptr(std::make_unique<mln::ping>(mln::ping(delta)));
    actions.emplace(ptr->get_command().name, std::move(ptr));
    //TODO i might be able to get the proper command snowflake id from certain methods (maybe in cluster?). With some roundabout logic a can use that to make maps with uint64_t instead of strings
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

    ptr = std::make_unique<mln::report>(mln::report(delta));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    delta->bot.on_slashcommand([this](const dpp::slashcommand_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event);
        }
    });
}
