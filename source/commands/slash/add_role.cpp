#include "commands/slash/add_role.h"
#include "commands/slash/base_slashcommand.h"
#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/json_err.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

mln::add_role::add_role(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("add_role"), "Give user a role", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
        .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true))
    ) } {}

dpp::job mln::add_role::command(dpp::slashcommand_t event_data) const {
    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed add_role, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_think(lite_data, true, false, {});

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(lite_data, "Failed add_role, discord error!", true, "Failed add_role, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    //Retrieve guild data
    const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_task(lite_data.guild_id, lite_data);
    if (!guild.has_value()) {
        co_return;
    }

    //Retrieve channel data
    const std::optional<std::shared_ptr<const dpp::channel>> channel = co_await mln::caches::get_channel_task(lite_data.channel_id, lite_data, &event_data.command.channel, &event_data.command.resolved.channels);
    if (!channel.has_value()) {
        co_return;
    }

    //Retrieve command user and bot information
    const std::optional<std::shared_ptr<const dpp::guild_member>> user = co_await mln::caches::get_member_task(lite_data.guild_id, lite_data.usr_id, lite_data, &event_data.command.member, &event_data.command.resolved.members);
    if (!user.has_value()) {
        co_return;
    }

    const std::optional<std::shared_ptr<const dpp::guild_member>> bot_opt = co_await mln::caches::get_member_task(lite_data.guild_id, lite_data.app_id, lite_data, &event_data.command.member, &event_data.command.resolved.members);
    if (!bot_opt.has_value()) {
        co_return;
    }

    //Retrieve user and bot perms, then return an error if the user and the bot don't have the required permissions
    const std::optional<dpp::permission> user_perm = co_await mln::perms::get_computed_permission_task(guild.value()->owner_id, *(channel.value()), *(user.value()), lite_data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
    if (!user_perm.has_value()) {
        co_return;
    }

    const std::optional<dpp::permission> bot_perm = co_await mln::perms::get_computed_permission_task(guild.value()->owner_id, *(channel.value()), *(bot_opt.value()), lite_data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
    if (!bot_perm.has_value()) {
        co_return;
    }

    //Check the most basic permission
    if (!mln::perms::check_permissions({user_perm.value(), bot_perm.value()}, dpp::permissions::p_manage_roles)) {
        co_await mln::response::co_respond(lite_data, "Failed command, either the user or the bot do not have the slashcommand required permission!", false, {});
        co_return;
    }

    const dpp::command_value& user_param = event_data.get_parameter("user");
    const dpp::command_value& role_param = event_data.get_parameter("role");
    const dpp::snowflake user_id = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ 0 };
    const dpp::snowflake role_id = std::holds_alternative<dpp::snowflake>(role_param) ? std::get<dpp::snowflake>(role_param) : dpp::snowflake{ 0 };

    if (user_id == 0) {
        co_await mln::response::co_respond(lite_data, "Failed to retrieve user parameter!", true, "Failed to retrieve user parameter!");
        co_return;
    }
    if (role_id == 0) {
        co_await mln::response::co_respond(lite_data, "Failed to retrieve role parameter!", true, "Failed to retrieve role parameter!");
        co_return;
    }

    //Retrieve command target information
    const std::optional<std::shared_ptr<const dpp::guild_member>> target = co_await mln::caches::get_member_task(lite_data.guild_id, user_id, lite_data, &event_data.command.member, &event_data.command.resolved.members);
    if (!target.has_value()) {
        co_return;
    }

    bool role_already_present = false;
    for (const dpp::snowflake& role : target.value()->get_roles()) {
        if (role == role_id) {
            role_already_present = true;
            break;
        }
    }

    if (role_already_present) {
        co_await mln::response::co_respond(lite_data, std::format("Failed to add role [{}] to [{}]! The user already has the given role.", dpp::role::get_mention(role_id), dpp::user::get_mention(target.value()->user_id)), false, {});
        co_return;
    }

    const dpp::confirmation_callback_t editing_user = co_await bot().co_guild_edit_member(dpp::guild_member{*target.value()}.add_role(role_id));

    if (editing_user.is_error()) {
        const dpp::error_info err = editing_user.get_error();
        const std::string err_text = std::format("Failed to add role [{}] to [{}]!", dpp::role::get_mention(role_id), dpp::user::get_mention(target.value()->user_id));

        co_await mln::response::co_respond(lite_data, err_text, true, std::format("{} Error: [{}], details: [{}].", err_text, mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    if (!std::holds_alternative<dpp::guild_member>(editing_user.value)) {
        static const std::string s_err_text = "Failed to confirm correct command execution!";

        co_await mln::response::co_respond(lite_data, s_err_text, true, std::format("{} Role: [{}, {}], user: [{}, {}].", 
            s_err_text, static_cast<uint64_t>(role_id), dpp::role::get_mention(role_id), static_cast<uint64_t>(target.value()->user_id), dpp::user::get_mention(target.value()->user_id)));
        co_return;
    }

    bool role_added = false;
    for (const dpp::snowflake& role : std::get<dpp::guild_member>(editing_user.value).get_roles()) {
        if (role == role_id) {
            role_added = true;
            break;
        }
    }
    if (role_added) {
        co_await mln::response::co_respond(lite_data, std::format("Role [{}] added to [{}]", dpp::role::get_mention(role_id), dpp::user::get_mention(target.value()->user_id)), false, "Failed add_role command conclusion reply!");
    }
    else {
        co_await mln::response::co_respond(lite_data, std::format("Failed to add role [{}, {}] to [{}, {}]!", 
            static_cast<uint64_t>(role_id), dpp::role::get_mention(role_id), static_cast<uint64_t>(target.value()->user_id), dpp::user::get_mention(target.value()->user_id)), false, "Failed add_role command conclusion reply!");
    }
    co_return;
}

std::optional<std::function<void()>> mln::add_role::job(dpp::slashcommand_t) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::add_role::use_job() const
{
    return false;
}
