#include "commands/slash/base_slashcommand.h"
#include "commands/slash/pm.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/json_err.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

static constexpr size_t s_extra_msg_size = 4;

static const size_t s_max_msg_size = mln::constants::get_max_characters_reply_msg() - mln::constants::get_max_nickname_length() - s_extra_msg_size;

mln::pm::pm(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("pm"), "Send a private message.", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "The user to message", true))
        .add_option(dpp::command_option(dpp::co_string, "msg", "The message to send", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(s_max_msg_size))))) } {
    cbot().log(dpp::loglevel::ll_debug, std::format("pm: [{}].", true));
}

dpp::job mln::pm::command(dpp::slashcommand_t event_data) {
    mln::event_data_lite_t lite_data{ event_data, bot(), false };

    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed to send pm, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_think(lite_data, true, false, {});

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(lite_data, "Failed to send pm, discord error!", true, "Failed to send pm, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    const dpp::command_value& user_param = event_data.get_parameter("user");
    const dpp::command_value& msg_param = event_data.get_parameter("msg");
    const dpp::snowflake user = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ 0 };
    const std::optional<std::string> msg = co_await mln::utility::check_text_validity(msg_param, lite_data, false,
        mln::constants::get_min_characters_reply_msg(), s_max_msg_size, "pm message");

    if (!msg.has_value()) {
        co_return;
    }

    if (user == 0) {
        co_await mln::response::co_respond(lite_data, "Failed to retrieve user parameter!", true, "Failed to retrieve user parameter!");
        co_return;
    }

    if (user == bot().me.id) {
        co_await mln::response::co_respond(lite_data, "Failed to open a DM channel with the given user, you cannot dm the bot!", false, {});
        co_return;
    }

    const dpp::confirmation_callback_t dir_msg_create_res = co_await bot().co_direct_message_create(user, dpp::message{ std::format("[{}]: {}", dpp::user::get_mention(lite_data.usr_id), msg.value())});
    
    if (dir_msg_create_res.is_error()) {
        const dpp::error_info err = dir_msg_create_res.get_error();

        co_await mln::response::co_respond(lite_data, "Failed to open a DM channel with the given user!", true,
            std::format("Failed to open a DM channel with the given user! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    co_await mln::response::co_respond(lite_data, "I've sent the target user a private message.", false, "Failed to reply with the pm text!");
}

std::optional<std::function<void()>> mln::pm::job(dpp::slashcommand_t)
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::pm::use_job() const noexcept
{
    return false;
}
