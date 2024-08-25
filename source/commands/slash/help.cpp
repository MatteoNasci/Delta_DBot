#include "commands/slash/help.h"
#include "bot_delta.h"
#include "utility/utility.h"
//TODO maybe use pagination (with emojis) rather than sending multiple embeds? With a timeout of like 60 seconds or something. I'm not sure if it's worth it tbh

mln::help::help(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("help", "Display information about this bot's commands.", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::command_option_type::co_boolean, "verbose", "Tells the bot to output as much info as possible about itself. Default: false", false)))) {}

dpp::task<void> mln::help::command(const dpp::slashcommand_t& event_data){
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);
    const dpp::command_value verbose_param = event_data.get_parameter("verbose");
    const bool verbose = std::holds_alternative<bool>(verbose_param) ? std::get<bool>(verbose_param) : false;
    
    co_await thinking;
    event_data.edit_response("The help info is currently being worked on.");
}