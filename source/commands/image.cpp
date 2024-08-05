#include "commands/image.h"

dpp::task<void> image::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Get the sub command */
    const dpp::command_interaction cmd_data = event.command.get_command_interaction();
    auto subcommand = cmd_data.options[0];
    /* Check if the subcommand is "dog" */
    if (subcommand.name == "dog") { 
        /* Checks if the subcommand has any options. */
        if (!subcommand.options.empty()) {
            /* Get the user from the parameter */
            dpp::user user = event.command.get_resolved_user(subcommand.get_value<dpp::snowflake>(0));
            event.reply(user.get_mention() + " has now been turned into a dog."); 
        } else {
            /* Reply if there were no options.. */
            event.reply("No user specified");
        }
    } else if (subcommand.name == "cat") { /* Check if the subcommand is "cat". */
        /* Checks if the subcommand has any options. */
        if (!subcommand.options.empty()) {
            /* Get the user from the parameter */
            dpp::user user = event.command.get_resolved_user(subcommand.get_value<dpp::snowflake>(0));
            event.reply(user.get_mention() + " has now been turned into a cat."); 
        } else {
            /* Reply if there were no options.. */
            event.reply("No user specified");
        }
    }
    co_return;
}
dpp::slashcommand image::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(image::get_command_name(), "Send a specific image.", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_sub_command, "dog", "Send a picture of a dog.").add_option(dpp::command_option(dpp::co_user, "user", "User to turn into a dog.", false)))
                    .add_option(dpp::command_option(dpp::co_sub_command, "cat", "Send a picture of a cat.").add_option(dpp::command_option(dpp::co_user, "user", "User to turn into a cat.", false)));
}
std::string image::get_command_name(){
    return "image";
}