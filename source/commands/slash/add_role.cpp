#include "commands/slash/add_role.h"
#include "bot_delta.h"

mln::add_role::add_role(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("add_role", "Give user a role", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
        .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast the role addition to the server. Default: false", false)))) {}

dpp::task<void> mln::add_role::command(const dpp::slashcommand_t& event_data){
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    const dpp::permission bot_perms = event_data.command.app_permissions;
    const dpp::permission& usr_perms = event_data.command.get_resolved_permission(event_data.command.usr.id);
    const bool are_perms_valid = bot_perms.can(dpp::permissions::p_manage_roles) && 
                                usr_perms.can(dpp::permissions::p_manage_roles);

    if (!are_perms_valid) {
        co_await thinking;
        event_data.edit_response("Failed to add new role to target, either the bot or the user do not possess the required permissions!");
        co_return;
    }

    const dpp::snowflake user_id = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    const dpp::snowflake role_id = std::get<dpp::snowflake>(event_data.get_parameter("role"));
    
    dpp::guild_member resolved_member = event_data.command.get_resolved_member(user_id);
    for (const dpp::snowflake& role : resolved_member.get_roles()) {
        if (role_id == role) {
            co_await thinking;
            event_data.edit_response("Failed to add new role to target, it already possesses the specified role!");
            co_return;
        }
    }

    resolved_member.add_role(role_id);

    const dpp::confirmation_callback_t editing_user = co_await delta()->bot.co_guild_edit_member(resolved_member);

    dpp::message msg{};
    if (editing_user.is_error()) {
        msg.set_content("Failed to add role " + dpp::role::get_mention(role_id) + " to " + dpp::user::get_mention(user_id) +". Error: " + editing_user.get_error().human_readable);
    }else {
        msg.set_content("Role " + dpp::role::get_mention(role_id) + " added to " + dpp::user::get_mention(user_id));
    }

    co_await thinking;
    event_data.edit_response(msg);
}