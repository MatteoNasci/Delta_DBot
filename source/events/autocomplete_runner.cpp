#include "events/autocomplete_runner.h"

mln::autocomplete_runner::autocomplete_runner() : actions({

	})
{
}

void mln::autocomplete_runner::attach_event(bot_delta_data_t& data){
    /* The on_autocomplete event is fired whenever discord needs information to fill in a command options's choices.
     * You must reply with a REST event within 500ms, so make it snappy!
     */
    data.bot.on_autocomplete([&data, this](const dpp::autocomplete_t& event) -> dpp::task<void> {
        const std::string key = event.name;
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            for (const auto& option : event.options) {
                if (option.focused) {
                    co_await function_it->second(data, event, option);
                    break;
                }
            }
        }
    });
}
