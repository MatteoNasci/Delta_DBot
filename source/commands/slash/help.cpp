#include "commands/slash/help.h"
#include "bot_delta.h"

mln::help::help(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("help", "Display information about this bot's commands.", delta->bot.me.id))) {}

dpp::job mln::help::command(dpp::slashcommand_t event){
    event.reply(dpp::message("WIP, will show information about the commands!").set_flags(dpp::m_ephemeral));
    co_return;
}