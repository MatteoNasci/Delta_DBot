#include "events/form_submit_runner.h"
#include "commands/dialog.h"
#include <variant>

form_submit_runner::form_submit_runner() : events(
        {{dialog::get_custom_id(), &dialog::form_command} }
)
{
    
}
void form_submit_runner::init(bot_delta_data_t& data)
{
    data.bot.on_form_submit([&data, this](const dpp::form_submit_t & event) -> dpp::task<void> {
        //base_runner::run(this->events, event.custom_id, data, event);
        const std::string key = event.custom_id;
        if (auto function_it = events.find(key); function_it != events.end()) {
            co_await function_it->second(data, event);
        }
    });
}