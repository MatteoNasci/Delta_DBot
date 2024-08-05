#include "commands/help.h"

dpp::task<void> help::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
    event.reply(dpp::message("WIP, will show information about the commands!").set_flags(dpp::m_ephemeral));
    co_return;
}
dpp::slashcommand help::get_command(dpp::cluster& bot){
    return dpp::slashcommand(help::get_command_name(), "Display information about this bot's commands.", bot.me.id);
}
std::string help::get_command_name(){
    return "help";
}