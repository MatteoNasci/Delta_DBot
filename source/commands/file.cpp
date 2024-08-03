#include "commands/file.h"


void file::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    dpp::message msg(event.command.channel_id, "");
 
    /* Attach the image to the message we just created. */
    msg.add_file("image.jpg", dpp::utility::read_file("D:\\Personal\\Projects\\DiscordBot\\assets\\dd6caac42dbccc609d66ee388b603118.jpg"));
    /* Create an embed. */

    dpp::embed embed;
    embed.set_image("attachment://image.jpg"); /* Set the image of the embed to the attached image. *
    /* Add the embed to the message. */
    msg.add_embed(embed);

    event.reply(msg);
}
dpp::slashcommand file::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(file::get_command_name(), "Send a local image along with an embed with the image!", bot.me.id);
}
std::string file::get_command_name(){
    return "file";
}