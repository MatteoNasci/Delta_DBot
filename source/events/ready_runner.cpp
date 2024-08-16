#include "events/ready_runner.h"
#include "commands/ready/register_commands.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>

void mln::ready_runner::attach_event(mln::bot_delta* const delta){
    actions.emplace_back(std::make_unique<mln::register_commands>(delta));

    delta->bot.on_ready([this](const dpp::ready_t& event) ->dpp::task<void> {
        for (const std::unique_ptr<mln::base_ready>& action : this->actions) {
            if (action->execute_command()) {
                co_await action->command(event);
            }
        }
    });
}
