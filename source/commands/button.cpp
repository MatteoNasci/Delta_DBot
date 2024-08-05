#include "commands/button.h"
#include <dpp/unicode_emoji.h>

dpp::task<void> button::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Create a message */
    dpp::message msg(event.command.channel_id, "this text has a button");
    /* Add an action row, and then a button within the action row. */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::smile)
                .set_style(dpp::cos_danger)
                .set_id("myid")
        )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
    co_return;
}
dpp::slashcommand button::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(button::get_command_name(), "Send a message with a button!", bot.me.id);
}
std::string button::get_command_name(){
    return "button";
}