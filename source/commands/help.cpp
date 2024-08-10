#include "commands/help.h"
#include "bot_delta.h"

dpp::task<void> mln::help::command(const dpp::slashcommand_t& event){
    event.reply(dpp::message("WIP, will show information about the commands!").set_flags(dpp::m_ephemeral));
    co_return;
}
dpp::slashcommand mln::help::get_command(){
    return dpp::slashcommand(mln::help::get_command_name(), "Display information about this bot's commands.", mln::bot_delta::delta().bot.me.id);
}
std::string mln::help::get_command_name(){
    return "help";
}