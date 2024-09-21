#include "commands/slash/pm.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "utility/response.h"
#include "utility/json_err.h"

#include <dpp/cluster.h>

mln::pm::pm(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("pm", "Send a private message.", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "The user to message", true))
        .add_option(dpp::command_option(dpp::co_string, "msg", "The message to send", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg()))))) } {}

dpp::task<void> mln::pm::command(const dpp::slashcommand_t& event_data) const {
    mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot());

    const dpp::snowflake user = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    const std::string msg = std::get<std::string>(event_data.get_parameter("msg"));

    if (user == bot().me.id) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Failed to open a DM channel with the given user, you cannot dm the bot!"), bot());
        co_return;
    }

    const dpp::confirmation_callback_t dir_msg_create_res = co_await bot().co_direct_message_create(user, dpp::message{ msg.empty() ? "Ping!" : msg });
    
    if (dir_msg_create_res.is_error()) {
        const dpp::error_info err = dir_msg_create_res.get_error();

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Failed to open a DM channel with the given user!"), bot(), &event_data, 
            std::format("Failed to open a DM channel with the given user! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "I've sent the target user a private message."), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the pm text!");
    }
}