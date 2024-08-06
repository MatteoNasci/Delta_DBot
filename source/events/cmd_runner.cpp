#include "events/cmd_runner.h"
#include "general/commands.h"
#include <dpp/cluster.h>

mln::cmd_runner::cmd_runner() : actions(
        {{mln::ping::get_command_name(), &mln::ping::command},
        {mln::bot_info::get_command_name(), &mln::bot_info::command},
        {mln::show::get_command_name(), &mln::show::command},
        {mln::pm::get_command_name(), &mln::pm::command},
        {mln::msgs_get::get_command_name(), &mln::msgs_get::command},
        {mln::add_role::get_command_name(), &mln::add_role::command},
        {mln::dialog::get_command_name(), &mln::dialog::command},
        {mln::add_emoji::get_command_name(), &mln::add_emoji::command},
        {mln::avatar::get_command_name(), &mln::avatar::command},
        {mln::co_button::get_command_name(), &mln::co_button::command},
        {mln::help::get_command_name(), &mln::help::command},
        {mln::report::get_command_name(), &mln::report::command} }
)
{
    
}

void mln::cmd_runner::attach_event(mln::bot_delta_data_t& data){
    data.bot.on_slashcommand([&data, this](const dpp::slashcommand_t& event) -> dpp::task<void> {
        const std::string key = event.command.get_command_name();
        if (auto function_it = actions.find(key); function_it != actions.end()) {
            co_await function_it->second(data, event);
        }
    });
}
