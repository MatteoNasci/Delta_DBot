#include "events/button_click_runner.h"

mln::button_click_runner::button_click_runner() : actions({

})
{
}

void mln::button_click_runner::attach_event(bot_delta_data_t& data){
    data.bot.on_button_click([&data, this](const dpp::button_click_t& event) -> dpp::task<void> {
        const std::string key = event.custom_id;
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second(data, event);
        }
    });
}
