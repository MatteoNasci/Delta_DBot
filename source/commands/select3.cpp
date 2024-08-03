#include "commands/select3.h"
#include <dpp/unicode_emoji.h>

void select3::select_command(bot_delta_data_t& data, const dpp::select_click_t& event){
    event.reply("You clicked " + event.custom_id + " and chose: " + event.values[0]);
}

void select3::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::message msg(event.command.channel_id, "this text has a button");
    /* Add an action row, and then a button within the action row. */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_label("Click me!")
                .set_type(dpp::cot_button)
                .set_emoji(dpp::unicode_emoji::smile)
                .set_style(dpp::cos_danger)
                .set_id(select3::get_custom_id())
        )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
}
dpp::slashcommand select3::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(select3::get_command_name(), "Select something at random!", bot.me.id);
}
std::string select3::get_command_name(){
    return "select3";
}
std::string select3::get_custom_id(){
    return "mysa2id";
}