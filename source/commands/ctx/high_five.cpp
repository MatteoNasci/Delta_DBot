#include "commands/ctx/high_five.h"
#include "bot_delta.h"

mln::high_five::high_five(mln::bot_delta* const delta) : base_ctx_command(delta,
    std::move(dpp::slashcommand("high_five!", "High five someone", delta->bot.me.id).set_type(dpp::ctxm_user))) {}

dpp::task<void> mln::high_five::command(const dpp::user_context_menu_t& event_data){
    const dpp::user user = event_data.get_user();
    const dpp::user author = event_data.command.get_issuing_user();
    event_data.reply(author.get_mention() + " slapped " + user.get_mention());
    co_return;
}