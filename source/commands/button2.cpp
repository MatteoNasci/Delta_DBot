#include "commands/button2.h"

void button2::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::message msg(event.command.channel_id, "This text has a select menu!");
    /* Add an action row, and a select menu within the action row. 
     *
     * Your default values are limited to max_values,
     * meaning you can't add more default values than the allowed max values.
     */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_type(dpp::cot_role_selectmenu)
                .set_min_values(2)
                .set_max_values(2)
                .add_default_value(dpp::snowflake{667756886443163648}, dpp::cdt_role)
                .set_id("myselect3id")
        )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
}
dpp::slashcommand button2::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(button2::get_command_name(), "Send a message with a button!", bot.me.id);
}
std::string button2::get_command_name(){
    return "button2";
}