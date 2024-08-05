#include "ctx/high_five.h"

dpp::task<void> high_five::ctx_command(bot_delta_data_t& data, const dpp::user_context_menu_t& event)
{
    dpp::user user = event.get_user(); // the user who the command has been issued on
    dpp::user author = event.command.get_issuing_user(); // the user who clicked on the context menu
    event.reply(author.get_mention() + " slapped " + user.get_mention());
    co_return;
}
dpp::slashcommand high_five::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(high_five::get_command_name(), "High five someone", bot.me.id).set_type(dpp::ctxm_user);
}
std::string high_five::get_command_name(){
    return "High Five";
}