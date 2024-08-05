#include "commands/add_role.h"

#include <dpp/snowflake.h>
#include <dpp/guild.h>

#include <variant>

dpp::task<void> add_role::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    /* Fetch a parameter value from the command options */
    dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("user"));
    dpp::snowflake role_id = std::get<dpp::snowflake>(event.get_parameter("role"));
    /* Get member object from resolved list */
    dpp::guild_member resolved_member = event.command.get_resolved_member(user_id);
    resolved_member.add_role(role_id);
    data.bot.guild_edit_member(resolved_member);
    event.reply("Added role");
    co_return;
}
dpp::slashcommand add_role::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(add_role::get_command_name(), "Give user a role", bot.me.id)
                /* Add user and role type command options to the slash command */
                .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
                .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true));
}
std::string add_role::get_command_name(){
    return "add_role";
}