#include "events/msg_react_remove_runner.h"

mln::msg_react_remove_runner::msg_react_remove_runner() : actions({

	})
{

}

void mln::msg_react_remove_runner::attach_event(bot_delta_data_t& data){
    data.bot.on_message_reaction_remove([&data, this](const dpp::message_reaction_remove_t& event) -> dpp::task<void> {
        for (const auto& func : this->actions) {
            co_await func(data, event);
        }
    });
}
