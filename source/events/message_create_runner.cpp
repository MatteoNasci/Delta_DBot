#include "events/message_create_runner.h"

mln::message_create_runner::message_create_runner() : actions({

	})
{
}

void mln::message_create_runner::attach_event(bot_delta_data_t& data){
    data.bot.on_message_create([&data, this](const dpp::message_create_t& event) -> dpp::task<void> {
        for (const auto& func : this->actions) {
            co_await func(data, event);
        }
    });
}
