#include "events/select_click_runner.h"

select_click_runner::select_click_runner() : events(
    {}
)
{
    
}

void select_click_runner::init(bot_delta_data_t& data)
{
    data.bot.on_select_click([&data, this](const dpp::select_click_t & event) -> dpp::task<void> {
        //base_runner::run(this->events, event.custom_id, data, event);
        const std::string key = event.custom_id;
        if(auto function_it = events.find(key); function_it != events.end()){
            co_await function_it->second(data, event);
        }
    });
}
