#include "events/cmd_ctx_runner.h"
#include "general/ctxs.h"
#include <dpp/cluster.h>

cmd_ctx_runner::cmd_ctx_runner() : events(
        {{high_five::get_command_name(), &high_five::ctx_command} }
)
{
    
}
void cmd_ctx_runner::init(bot_delta_data_t& data)
{
    data.bot.on_user_context_menu([&data, this](const dpp::user_context_menu_t& event) -> dpp::task<void> {
        //base_runner::run(this->events, event.command.get_command_name(), data, event);
        const std::string key = event.command.get_command_name();
        if (auto function_it = events.find(key); function_it != events.end()) {
            co_await function_it->second(data, event);
        }
    });
}