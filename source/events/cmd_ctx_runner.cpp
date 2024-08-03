#include "events/cmd_ctx_runner.h"
#include "general/ctxs.h"
#include <dpp/cluster.h>
#include "events/base_runner.h"

cmd_ctx_runner::cmd_ctx_runner() : events(
        {{high_five::get_command_name(), &high_five::ctx_command} }
)
{
    
}
void cmd_ctx_runner::init(bot_delta_data_t& data)
{
    data.bot.on_user_context_menu([&data, this](const dpp::user_context_menu_t& event) {
        base_runner::run(this->events, event.command.get_command_name(), data, event);
    });
}