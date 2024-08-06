#include "events/ready_runner.h"

#include "ready/register_commands.h"

mln::ready_runner::ready_runner() : actions(
    { { std::make_pair<ready_condition, ready_action>(&mln::register_commands::execute_command, &mln::register_commands::command) } }
)
{

}

void mln::ready_runner::attach_event(mln::bot_delta_data_t& data){
    data.bot.on_ready([&data, this](const dpp::ready_t& event) -> dpp::task<void> {
        for (const auto& pair : this->actions) {
            if (pair.first(data)) {
                co_await pair.second(data, event);
            }
        }
    });
}
