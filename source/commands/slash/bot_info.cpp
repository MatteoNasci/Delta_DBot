#include "commands/slash/bot_info.h"
#include "bot_delta.h"
#include "version.h"

#include <dpp/colors.h>

mln::bot_info::bot_info(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("info", "Send an embed with the bot info!", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands))) {}

dpp::task<void> mln::bot_info::command(const dpp::slashcommand_t& event_data){
    static const dpp::embed s_embed = dpp::embed{}
        .set_color(dpp::colors::sti_blue)
        .set_title("Delta")
        .set_author("Erk_Krea", "https://github.com/MatteoNasci/Delta_DBot", "https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_description("Bot version: " + std::string(mln::get_version()))
        .set_thumbnail("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_image("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_timestamp(time(0));

    event_data.reply(dpp::message{event_data.command.channel_id, dpp::embed{s_embed}}.set_flags(dpp::m_ephemeral));
    co_return;
}