#include "events/select_click_runner.h"
#include "commands/select.h"
#include "commands/select2.h"
#include "commands/select3.h"

select_click_runner::select_click_runner() : events(
    {{select::get_custom_id(), &select::select_command},
    {select2::get_custom_id(), &select2::select_command},
    {select3::get_custom_id(), &select3::select_command}}
)
{
    
}

void select_click_runner::init(bot_delta_data_t& data)
{
    data.bot.on_select_click([&data, this](const dpp::select_click_t & event) -> dpp::task<void> {
        //base_runner::run(this->events, event.custom_id, data, event);
        const auto key = event.custom_id;
        if(events.contains(key)){
            co_await events.at(key)(data, event);
        }
    });
}
