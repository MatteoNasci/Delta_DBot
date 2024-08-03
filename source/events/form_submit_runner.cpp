#include "events/form_submit_runner.h"
#include "commands/dialog.h"
#include <variant>
#include "events/base_runner.h"

form_submit_runner::form_submit_runner() : events(
        {{dialog::get_custom_id(), &dialog::form_command} }
)
{
    
}
void form_submit_runner::init(bot_delta_data_t& data)
{
    data.bot.on_form_submit([&data, this](const dpp::form_submit_t & event) {
        base_runner::run(this->events, event.custom_id, data, event);
    });
}