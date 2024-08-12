#include "events/ready_runner.h"
#include "commands/ready/register_commands.h"
#include "bot_delta.h"

#include <dpp/dispatcher.h>

void mln::ready_runner::attach_event(bot_delta* const delta){
    actions.emplace_back(std::make_unique<mln::register_commands>(delta));

    delta->bot.on_ready([this](const dpp::ready_t& event) ->dpp::job {
        std::shared_ptr<dpp::ready_t> ptr(std::make_shared<dpp::ready_t>(event));
        for (const auto& action : this->actions) {
            if (action->execute_command()) {
                action->command(ptr);
            }
        }
        co_return;
    });
}
