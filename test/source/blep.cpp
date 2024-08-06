#include "commands/blep.h"
#include <variant>

dpp::task<void> blep::autocomplete_command(bot_delta_data_t& data, const dpp::autocomplete_t event, const dpp::command_option& focused_opt) {
    /* In a real world usage of this function you should return values that loosely match
    * opt.value, which contains what the user has typed so far. The opt.value is a variant
    * and will contain the type identical to that of the slash command parameter.
    * Here we can safely know it is string.
    */
    std::string uservalue = std::get<std::string>(opt.value);
    delta.data.bot.interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply)
        .add_autocomplete_choice(dpp::command_option_choice("squids", std::string("lots of squids")))
        .add_autocomplete_choice(dpp::command_option_choice("cats", std::string("a few cats")))
        .add_autocomplete_choice(dpp::command_option_choice("dogs", std::string("bucket of dogs")))
        .add_autocomplete_choice(dpp::command_option_choice("elephants", std::string("bottle of elephants")))
    );
    delta.data.bot.log(dpp::ll_debug, "Autocomplete " + opt.name + " with value '" + uservalue + "' in field " + event.name);
}
dpp::task<void> blep::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Fetch a parameter value from the command parameters */
    std::string animal = std::get<std::string>(event.get_parameter("animal"));
    /* Reply to the command. There is an overloaded version of this
    * call that accepts a dpp::message so you can send embeds.
    */
    event.reply("Blep! You chose " + animal);
    co_return;
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