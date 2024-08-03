#include "commands/pm.h"

void pm::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::snowflake user;
 
    /* If there was no specified user, we set the "user" variable to the command author (issuing user). */
    if (event.get_parameter("user").index() == 0) {
        user = event.command.get_issuing_user().id;
    } else { /* Otherwise, we set it to the specified user! */
        user = std::get<dpp::snowflake>(event.get_parameter("user"));
    }
    std::string msg;
    if (event.get_parameter("msg").index() != 0) {
        msg = std::get<std::string>(event.get_parameter("msg"));
    }
    /* Send a message to the user set above. */
    data.bot.direct_message_create(user, dpp::message(msg.empty() ? "Ping!" : msg), [event, user](const dpp::confirmation_callback_t& callback){
        /* If the callback errors, we want to send a message telling the author that something went wrong. */
        if (callback.is_error()) {
            /* Here, we want the error message to be different if the user we're trying to send a message to is the command author. */
            if (user == event.command.get_issuing_user().id) {
                event.reply(dpp::message("I couldn't send you a message.").set_flags(dpp::m_ephemeral));
            } else {
                event.reply(dpp::message("I couldn't send a message to that user. Please check that is a valid user!").set_flags(dpp::m_ephemeral));
            }
            return;
        }
        /* We do the same here, so the message is different if it's to the command author or if it's to a specified user. */
        if (user == event.command.get_issuing_user().id) {
            event.reply(dpp::message("I've sent you a private message.").set_flags(dpp::m_ephemeral));
        } else {
            event.reply(dpp::message("I've sent a message to that user.").set_flags(dpp::m_ephemeral));
        }
    });
}
dpp::slashcommand pm::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(pm::get_command_name(), "Send a private message.", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_mentionable, "user", "The user to message", false))
                    .add_option(dpp::command_option(dpp::co_string, "msg", "The message to send", false));
}
std::string pm::get_command_name(){
    return "pm";
}