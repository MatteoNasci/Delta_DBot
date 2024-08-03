#include "commands/blep.h"
#include <variant>

void blep::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Fetch a parameter value from the command parameters */
    std::string animal = std::get<std::string>(event.get_parameter("animal"));
    /* Reply to the command. There is an overloaded version of this
    * call that accepts a dpp::message so you can send embeds.
    */
    event.reply("Blep! You chose " + animal);
}
dpp::slashcommand blep::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(blep::get_command_name(), "Send a random adorable animal photo", bot.me.id)
                /* If you set the auto complete setting on a command option, it will trigger the on_autocomplete
                * event whenever discord needs to fill information for the choices. You cannot set any choices
                * here if you set the auto complete value to true.
                */
                .add_option(dpp::command_option(dpp::co_string, "animal", "The type of animal").set_auto_complete(true));
}
std::string blep::get_command_name(){
    return "blep";
}