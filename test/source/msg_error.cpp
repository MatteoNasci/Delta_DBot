#include "commands/msg_error.h"

dpp::task<void> msg_error::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    auto msg_get_res = co_await data.bot.co_message_get(0, 0);

    /* the error will occur since there is no message with ID '0' that is in a channel with ID '0' (I'm not explaining why) */
    if (msg_get_res.is_error()) {
        event.reply(msg_get_res.get_error().message);
        co_return;
    }
    /* we won't be able to get here because of the return; statement */
    auto message = msg_get_res.get<dpp::message>();
    event.reply(message);
}
dpp::slashcommand msg_error::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(msg_error::get_command_name(), "Get an error instead of message :)", bot.me.id);
}
std::string msg_error::get_command_name(){
    return "msg_error";
}