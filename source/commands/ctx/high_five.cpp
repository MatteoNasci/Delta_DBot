#include "commands/ctx/high_five.h"
#include "bot_delta.h"

mln::high_five::high_five(mln::bot_delta* const delta) : base_ctx_command(delta,
    std::move(dpp::slashcommand("high_five!", "High five someone", delta->bot.me.id).set_type(dpp::ctxm_user))) {}

dpp::job mln::high_five::command(dpp::user_context_menu_t event){
    dpp::user user = event.get_user();
    dpp::user author = event.command.get_issuing_user();
    event.reply(author.get_mention() + " slapped " + user.get_mention());
    co_return;
}