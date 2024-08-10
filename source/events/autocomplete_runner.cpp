#include "events/autocomplete_runner.h"
#include "bot_delta.h"

mln::autocomplete_runner::autocomplete_runner() : actions({

	})
{
}

void mln::autocomplete_runner::attach_event(){
    /* The on_autocomplete event is fired whenever discord needs information to fill in a command options's choices.
     * You must reply with a REST event within 500ms, so make it snappy!
     */
    mln::bot_delta::delta().bot.on_autocomplete([this](const dpp::autocomplete_t& event) -> dpp::task<void> {
        const std::string key = event.name;
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            for (const auto& option : event.options) {
                if (option.focused) {
                    co_await function_it->second(event, option);
                    break;
                }
            }
        }
    });
}
