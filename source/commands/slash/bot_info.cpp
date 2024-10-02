#include "commands/slash/base_slashcommand.h"
#include "commands/slash/bot_info.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"
#include "version.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/permissions.h>

#include <ctime>
#include <format>
#include <functional>
#include <optional>
#include <type_traits>

mln::bot_info::bot_info(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("info"), "Send an embed with the bot info!", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {}

dpp::job mln::bot_info::command(dpp::slashcommand_t event_data) const {
    static const dpp::embed s_embed = dpp::embed{}
        .set_color(dpp::colors::sti_blue)
        .set_title("Delta")
        .set_author("Erk_Krea", "https://github.com/MatteoNasci/Delta_DBot", "https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_thumbnail("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_image("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_timestamp(time(0));

    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed info, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_respond(lite_data, dpp::message{
            lite_data.channel_id,
            dpp::embed{s_embed}.set_description(std::format("Bot version: [{}], uptime: [{}]", mln::get_version(), bot().uptime().to_string())) }
    .set_flags(dpp::m_ephemeral), false, "Failed to reply with the info text!");

    co_return;
}

std::optional<std::function<void()>> mln::bot_info::job(dpp::slashcommand_t) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::bot_info::use_job() const
{
    return false;
}
