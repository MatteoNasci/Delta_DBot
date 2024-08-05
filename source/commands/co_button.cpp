#include "commands/co_button.h"
#include <dpp/message.h>
#include <dpp/coro/when_any.h>

dpp::task<void> co_button::command(bot_delta_data_t &data, const dpp::slashcommand_t &event){
    std::string id{event.command.id.str()};
    co_await event.co_reply(dpp::message{"Test"}
            .add_component(dpp::component{}
            .add_component(dpp::component{}
            .set_type(dpp::cot_button)
            .set_label("Click me!")
            .set_id(id))
    ));

    auto result = co_await dpp::when_any{ 
        event.from->creator->on_button_click.when([id](const dpp::button_click_t &b) {
            return b.custom_id == id;
        }),
        event.from->creator->co_sleep(5) // Or sleep 5 seconds
    };

    if (result.index() == 0) { // Awaitable #0 completed first, that is the button click event
        // Acknowledge the click and edit the original response, removing the button
        const dpp::button_click_t &click_event = result.get<0>();
        click_event.reply();
        event.edit_original_response(dpp::message{"You clicked the button with the id " + click_event.custom_id});
    } else { //index() is 1, the timer expired
        event.edit_original_response(dpp::message{"I haven't got all day!"});
    }
}

dpp::slashcommand co_button::get_command(dpp::cluster &bot){
    return dpp::slashcommand(co_button::get_command_name(), "Test awaiting for an event", bot.me.id);
}

std::string co_button::get_command_name(){
    return "co_button";
}
