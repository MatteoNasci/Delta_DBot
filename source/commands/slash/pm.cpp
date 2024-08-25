#include "commands/slash/pm.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::pm::pm(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("pm", "Send a private message.", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "The user to message", true))
        .add_option(dpp::command_option(dpp::co_string, "msg", "The message to send", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg())))))){}

dpp::task<void> mln::pm::command(const dpp::slashcommand_t& event_data){
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    const dpp::snowflake user = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    const std::string msg = std::get<std::string>(event_data.get_parameter("msg"));

    if (user == delta()->bot.me.id) {
        co_await thinking;
        event_data.edit_response("Failed to open a DM channel with the given user, you cannot dm the bot!");
        co_return;
    }

    const dpp::confirmation_callback_t dir_msg_create_res = co_await delta()->bot.co_direct_message_create(user, dpp::message(msg.empty() ? "Ping!" : msg));
    
    if (dir_msg_create_res.is_error()) {
        co_await thinking;
        event_data.edit_response("Failed to open a DM channel with the given user! Error: " + dir_msg_create_res.get_error().human_readable);
        co_return;
    }

    co_await thinking;
    event_data.edit_response(dpp::message("I've sent the target user a private message."));
}