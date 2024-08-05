#include "events/cmd_runner.h"
#include "general/commands.h"
#include <dpp/cluster.h>

cmd_runner::cmd_runner() : events(
        {{ping::get_command_name(), &ping::command},
        {bot_info::get_command_name(), &bot_info::command},
        {show::get_command_name(), &show::command},
        {pm::get_command_name(), &pm::command},
        {msgs_get::get_command_name(), &msgs_get::command},
        {add_role::get_command_name(), &add_role::command},
        {dialog::get_command_name(), &dialog::command},
        {add_emoji::get_command_name(), &add_emoji::command},
        {avatar::get_command_name(), &avatar::command},
        {co_button::get_command_name(), &co_button::command},
        {help::get_command_name(), &help::command} }
)
{
    
}

void cmd_runner::init(bot_delta_data_t& data)
{
    data.bot.on_slashcommand([&data, this](const dpp::slashcommand_t& event) -> dpp::task<void> {
        //base_runner::run(this->events, event.command.get_command_name(), data, event);
        const std::string key = event.command.get_command_name();
        if (auto function_it = events.find(key); function_it != events.end()) {
            co_await function_it->second(data, event);
        }
    });
}
