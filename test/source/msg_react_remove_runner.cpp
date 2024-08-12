#include "events/msg_react_remove_runner.h"
#include "bot_delta.h"

mln::msg_react_remove_runner::msg_react_remove_runner() : actions({

	})
{

}

void mln::msg_react_remove_runner::attach_event(){
    delta()->bot.on_message_reaction_remove([this](const dpp::message_reaction_remove_t& event) -> dpp::task<void> {
        for (const auto& func : this->actions) {
            co_await func(event);
        }
    });
}
