#include "commands/msg_error.h"

void msg_error::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    data.bot.message_get(0, 0, [event](const dpp::confirmation_callback_t& callback) -> void {
        /* the error will occur since there is no message with ID '0' that is in a channel with ID '0' (I'm not explaining why) */
        if (callback.is_error()) {
            event.reply(callback.get_error().message);
            return;
        }
        /* we won't be able to get here because of the return; statement */
        auto message = callback.get<dpp::message>();
        event.reply(message);
    });
}
dpp::slashcommand msg_error::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(msg_error::get_command_name(), "Get an error instead of message :)", bot.me.id);
}
std::string msg_error::get_command_name(){
    return "msg_error";
}