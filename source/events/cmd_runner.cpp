#include "events/cmd_runner.h"
#include "general/commands.h"
#include <dpp/cluster.h>
#include "events/base_runner.h"

cmd_runner::cmd_runner() : events(
        {{ping::get_command_name(), &ping::command},
        {pong::get_command_name(), &pong::command},
        {thinking::get_command_name(), &thinking::command},
        {bot_info::get_command_name(), &bot_info::command},
        {file::get_command_name(), &file::command},
        {pm::get_command_name(), &pm::command},
        {msgs_get::get_command_name(), &msgs_get::command},
        {channel_create::get_command_name(), &channel_create::command},
        {msg_error::get_command_name(), &msg_error::command},
        {image::get_command_name(), &image::command},
        {blep::get_command_name(), &blep::command},
        {show::get_command_name(), &show::command},
        {add_role::get_command_name(), &add_role::command},
        {button::get_command_name(), &button::command},
        {button2::get_command_name(), &button2::command},
        {math::get_command_name(), &math::command},
        {pring::get_command_name(), &pring::command},
        {select::get_command_name(), &select::command},
        {select2::get_command_name(), &select2::command},
        {select3::get_command_name(), &select3::command},
        {dialog::get_command_name(), &dialog::command} }
)
{
    
}

void cmd_runner::init(bot_delta_data_t& data)
{
    data.bot.on_slashcommand([&data, this](const dpp::slashcommand_t& event) {
        base_runner::run(this->events, event.command.get_command_name(), data, event);
    });
}
