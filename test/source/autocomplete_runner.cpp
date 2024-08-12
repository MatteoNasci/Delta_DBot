#include "events/autocomplete_runner.h"
#include "bot_delta.h"

mln::autocomplete_runner::autocomplete_runner() : base_event()
{

}

void mln::autocomplete_runner::attach_event(){
    /* The on_autocomplete event is fired whenever discord needs information to fill in a command options's choices.
     * You must reply with a REST event within 500ms, so make it snappy!
     */
    delta()->bot.on_autocomplete([this](const dpp::autocomplete_t& event) -> dpp::task<void> {
        dpp::autocomplete_t copied_event = event;
        const uint64_t key = copied_event.id;
        if (const auto& function_it = actions.find(key); function_it != actions.end()) {
            for (const auto& option : copied_event.options) {
                if (option.focused) {
                    co_await function_it->second->command(copied_event, option);
                    break;
                }
            }
        }
    });//TODO it might be better to save the event locally (for all event runners) and then pass the local event to the actions, so I am sure that the event data doesn't get destroyed while the actions are co_awaiting
}
