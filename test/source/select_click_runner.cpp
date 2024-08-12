#include "events/select_click_runner.h"
#include "bot_delta.h"

mln::select_click_runner::select_click_runner() : actions(
    {}
)
{
    
}

void mln::select_click_runner::attach_event()
{
    delta()->bot.on_select_click([ this](const dpp::select_click_t & event) -> dpp::task<void> {
        const std::string key = event.custom_id;
        if(auto function_it = actions.find(key); function_it != actions.end()){
            co_await function_it->second(event);
        }
    });
}
