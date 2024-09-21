#include "commands/slash/bot_info.h"
#include "version.h"
#include "utility/utility.h"

#include <dpp/colors.h>
#include <dpp/cluster.h>

mln::bot_info::bot_info(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("info", "Send an embed with the bot info!", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {}

dpp::task<void> mln::bot_info::command(const dpp::slashcommand_t& event_data) const {
    static const dpp::embed s_embed = dpp::embed{}
        .set_color(dpp::colors::sti_blue)
        .set_title("Delta")
        .set_author("Erk_Krea", "https://github.com/MatteoNasci/Delta_DBot", "https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_description(std::format("Bot version: [{}], uptime: [{}]", mln::get_version(), bot().uptime().to_string()))
        .set_thumbnail("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_image("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_timestamp(time(0));

    if (mln::utility::conf_callback_is_error(co_await event_data.co_reply(dpp::message{ event_data.command.channel_id, dpp::embed{s_embed} }.set_flags(dpp::m_ephemeral)), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the db help text!");
    }
    co_return;
}