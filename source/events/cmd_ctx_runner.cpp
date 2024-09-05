#include "events/cmd_ctx_runner.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>
#include <dpp/coro/job.h>

void mln::cmd_ctx_runner::attach_event(mln::bot_delta* const delta){
    //No commands at the moment

    delta->bot.on_user_context_menu([this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second->command(event);
        }
    });
}