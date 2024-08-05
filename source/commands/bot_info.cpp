#include "commands/bot_info.h"
#include <dpp/colors.h>

dpp::task<void> bot_info::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
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
    /* Create a message with the content as our new embed. */
    dpp::message msg(event.command.channel_id, embed);
    /* Reply to the user with the message, containing our embed. */
    event.reply(msg);
    co_return;
}
dpp::slashcommand bot_info::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(bot_info::get_command_name(), "Send an embed with the bot info!", bot.me.id);
}
std::string bot_info::get_command_name(){
    return "bot_info";
}