#include "events/cmd_ctx_runner.h"
#include "commands/ctx/high_five.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/job.h>

void mln::cmd_ctx_runner::attach_event(mln::bot_delta* const delta){
    std::unique_ptr<mln::base_ctx_command> ptr(std::make_unique<mln::high_five>(mln::high_five(delta)));
    actions.emplace(ptr->get_command().name, std::move(ptr));

    delta->bot.on_user_context_menu([this](const dpp::user_context_menu_t& event) -> dpp::job {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            function_it->second->command(event);
        }
        co_return;
    });
}