#include "events/cmd_ctx_runner.h"
#include "general/ctxs.h"
#include <dpp/cluster.h>

mln::cmd_ctx_runner::cmd_ctx_runner() : actions(
        {{mln::high_five::get_command_name(), &mln::high_five::ctx_command} }
)
{
    
}
void mln::cmd_ctx_runner::attach_event(mln::bot_delta_data_t& data)
{
    data.bot.on_user_context_menu([&data, this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second(data, event);
        }
    });
}