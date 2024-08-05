#include "commands/select2.h"

dpp::task<void> select2::select_command(bot_delta_data_t& data, const dpp::select_click_t& event){
    event.reply("You clicked " + event.custom_id + " and chose: " + event.values[0]);
    co_return;
}
dpp::task<void> select2::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::message msg(event.command.channel_id, "This text has a select menu!");
        
    /* Add an action row, and a select menu within the action row. 
     *
     * By default, max values is 1, meaning people can only pick 1 option.
     * We're changing this to two, so people can select multiple options!
     * We'll also set the min_values to 2 so people have to pick another value!
     */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_type(dpp::cot_role_selectmenu)
                .set_min_values(2)
                .set_max_values(2)
                .set_id(select2::get_custom_id())
        )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
    co_return;
}
dpp::slashcommand select2::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(select2::get_command_name(), "Select something at random!", bot.me.id);
}
std::string select2::get_command_name(){
    return "select2";
}
std::string select2::get_custom_id(){
    return "myselect2id";
}