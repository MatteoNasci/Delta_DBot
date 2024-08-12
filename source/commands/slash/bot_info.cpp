#include "commands/slash/bot_info.h"
#include "bot_delta.h"

#include <dpp/colors.h>

mln::bot_info::bot_info(bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("bot_info", "Send an embed with the bot info!", delta->bot.me.id))) {}

dpp::job mln::bot_info::command(dpp::slashcommand_t event){
    dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::sti_blue)
        .set_title("Delta")
        .set_author("Erk_Krea", "https://github.com/MatteoNasci/Delta_DBot", "https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_description("Bot description")
        .set_thumbnail("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .add_field(
            "Regular field title",
            "Some value here"
        )
        .add_field(
            "Inline field title",
            "Some value here",
            true
        )
        .add_field(
            "Inline field title",
            "Some value here",
            true
        )
        .set_image("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        .set_footer(
            dpp::embed_footer()
            .set_text("Some footer text here")
            .set_icon("https://avatars.githubusercontent.com/u/28777038?s=40&v=64")
        )
        .set_timestamp(time(0));

    dpp::message msg(event.command.channel_id, embed);

    event.reply(msg.set_flags(dpp::m_ephemeral));
    co_return;
}