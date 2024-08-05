#include "commands/pring.h"
#include <variant>

dpp::task<void> pring::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    std::string msg;
    if (event.get_parameter("testparameter").index() != 0) {
        msg = std::get<std::string>(event.get_parameter("testparameter"));
    }
    event.reply("Prong! -> " + msg);
    co_return;
}
dpp::slashcommand pring::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(pring::get_command_name(), "A test ping command", bot.me.id)
                .add_option(dpp::command_option(dpp::co_string, "testparameter", "Optional test parameter"));
}
std::string pring::get_command_name(){
    return "pring";
}