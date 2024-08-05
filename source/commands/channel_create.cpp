#include "commands/channel_create.h"

dpp::task<void> channel_create::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* create a text channel */
    dpp::channel channel = dpp::channel()
        .set_name("test")
        .set_guild_id(event.command.guild_id);

    auto creating_channel = co_await data.bot.co_channel_create(channel);

    if (creating_channel.is_error()) { /* catching an error to log it */
        data.bot.log(dpp::loglevel::ll_error, creating_channel.get_error().message);
        co_return;
    }
    /* std::get<dpp::channel>(callback.value) would give the same result */
    /* reply with the created channel information */
    dpp::message message = dpp::message("The channel's name is `" + channel.name + "`, ID is `" + std::to_string(channel.id) + " and type is `" + std::to_string(channel.get_type()) + "`.");
    /* note that channel types are represented as numbers */
    event.reply(message);

}
dpp::slashcommand channel_create::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(channel_create::get_command_name(), "Create a channel", bot.me.id);
}
std::string channel_create::get_command_name(){
    return "channel_create";
}