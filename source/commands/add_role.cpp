#include "commands/add_role.h"

#include <dpp/snowflake.h>
#include <dpp/guild.h>

#include <variant>

dpp::task<void> add_role::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
    dpp::snowflake user_id = std::get<dpp::snowflake>(event.get_parameter("user"));
    dpp::snowflake role_id = std::get<dpp::snowflake>(event.get_parameter("role"));
    const bool broadcast_role_change = !(std::get<dpp::snowflake>(event.get_parameter("broadcast")).empty());

    dpp::guild_member resolved_member = event.command.get_resolved_member(user_id);
    resolved_member.add_role(role_id);

    dpp::confirmation_callback_t editing_user = co_await data.bot.co_guild_edit_member(resolved_member);

    dpp::message msg;
    if (editing_user.is_error()) {
        msg.set_content("Failed to add role [" + dpp::role::get_mention(role_id) + "] to " + dpp::user::get_mention(user_id));
    }else {
        msg.set_content("Role [" + dpp::role::get_mention(role_id) + "] added to " + dpp::user::get_mention(user_id));
    }

    if (!broadcast_role_change) {
        msg = msg.set_flags(dpp::m_ephemeral);
    }
    event.reply(msg);
    co_return;
}
dpp::slashcommand add_role::get_command(dpp::cluster& bot){
    return dpp::slashcommand(add_role::get_command_name(), "Give user a role", bot.me.id)
            .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
            .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true))
            .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast the role addition to the server. Default: false", false));
}
std::string add_role::get_command_name(){
    return "add_role";
}