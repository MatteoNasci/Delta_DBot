#include "ctx/high_five.h"
#include "bot_delta.h"

dpp::task<void> mln::high_five::ctx_command(const dpp::user_context_menu_t& event){
    dpp::user user = event.get_user();
    dpp::user author = event.command.get_issuing_user();
    event.reply(author.get_mention() + " slapped " + user.get_mention());
    co_return;
}
dpp::slashcommand mln::high_five::get_command(){
    return dpp::slashcommand(mln::high_five::get_command_name(), "High five someone", mln::bot_delta::delta().bot.me.id).set_type(dpp::ctxm_user);
}
std::string mln::high_five::get_command_name(){
    return "High Five";
}