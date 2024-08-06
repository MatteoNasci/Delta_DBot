#include "events/form_submit_runner.h"

#include <variant>

mln::form_submit_runner::form_submit_runner() : actions({
    }
)
{
    
}
void mln::form_submit_runner::attach_event(mln::bot_delta_data_t& data)
{
    data.bot.on_form_submit([&data, this](const dpp::form_submit_t & event) -> dpp::task<void> {
        const std::string key = event.custom_id;
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second(data, event);
        }
    });
}