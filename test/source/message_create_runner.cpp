#include "events/message_create_runner.h"
#include "bot_delta.h"

mln::message_create_runner::message_create_runner() : actions({

	})
{
}

void mln::message_create_runner::attach_event(){
    delta()->bot.on_message_create([this](const dpp::message_create_t& event) -> dpp::task<void> {
        for (const auto& func : this->actions) {
            co_await func(event);
        }
    });
}
