#include "commands/math.h"

void math::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Create a message */
    dpp::message msg(event.command.channel_id, "What is 5+5?");
    /* Add an action row, and then 3 buttons within the action row. */
    msg.add_component(
        dpp::component().add_component(
            dpp::component()
                .set_label("9")
                .set_style(dpp::cos_primary)
                .set_id("9")
        )
        .add_component(
            dpp::component()
                .set_label("10")
                .set_style(dpp::cos_primary)
                .set_id("10")
        )
        .add_component(
            dpp::component()
                .set_label("11")
                .set_style(dpp::cos_primary)
                .set_id("11")
        )
    );
    /* Reply to the user with our message. */
    event.reply(msg);
}
dpp::slashcommand math::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(math::get_command_name(), "A quick maths question!", bot.me.id);
}
std::string math::get_command_name(){
    return "math";
}