#include "commands/select.h"
#include <dpp/unicode_emoji.h>

dpp::task<void> select::select_command(bot_delta_data_t& data, const dpp::select_click_t& event){
    /* Select clicks are still interactions, and must be replied to in some form to
    * prevent the "this interaction has failed" message from Discord to the user.
    */
    event.reply("You clicked " + event.custom_id + " and chose: " + event.values[0]);
    co_return;
}
dpp::task<void> select::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::message msg(event.command.channel_id, "This text has a select menu!");

    /* Add an action row, and a select menu within the action row. */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_type(dpp::cot_selectmenu)
                .set_placeholder("Pick something")
                .add_select_option(dpp::select_option("label1","value1","description1").set_emoji(dpp::unicode_emoji::smile))
                .add_select_option(dpp::select_option("label2","value2","description2").set_emoji(dpp::unicode_emoji::slight_smile))
                .set_id(select::get_custom_id())
            )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
    co_return;
}
dpp::slashcommand select::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(select::get_command_name(), "Select something at random!", bot.me.id);
}
std::string select::get_command_name(){
    return "select";
}
std::string select::get_custom_id(){
    return "myselectid";
}