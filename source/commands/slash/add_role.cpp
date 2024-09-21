#include "commands/slash/add_role.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "utility/response.h"
#include "utility/json_err.h"
#include "utility/reply_log_data.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

mln::add_role::add_role(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("add_role", "Give user a role", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
        .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true))
    ) } {}

dpp::task<void> mln::add_role::command(const dpp::slashcommand_t& event_data) const {
    mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot());

    const reply_log_data_t reply_log_data{ &event_data, &bot(), false };
    //Retrieve guild data
    const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_full(event_data.command.guild_id, reply_log_data);
    if (!guild.has_value()) {
        co_return;
    }

    //Retrieve channel data
    const std::optional<std::shared_ptr<const dpp::channel>> channel = co_await mln::caches::get_channel_full(event_data.command.channel_id, reply_log_data);
    if (!channel.has_value()) {
        co_return;
    }

    //Retrieve command user and bot information
    const std::optional<std::shared_ptr<const dpp::guild_member>> user = co_await mln::caches::get_member_full({guild.value()->id, event_data.command.usr.id}, reply_log_data);
    if (!user.has_value()) {
        co_return;
    }

    const std::optional<std::shared_ptr<const dpp::guild_member>> bot_opt = co_await mln::caches::get_member_full({guild.value()->id, event_data.command.application_id}, reply_log_data);
    if (!bot_opt.has_value()) {
        co_return;
    }

    //Retrieve user and bot perms, then return an error if the user and the bot don't have the required permissions
    const std::optional<dpp::permission> user_perm = co_await mln::perms::get_computed_permission_full(*(guild.value()), *(channel.value()), *(user.value()), reply_log_data);
    if (!user_perm.has_value()) {
        co_return;
    }

    const std::optional<dpp::permission> bot_perm = co_await mln::perms::get_computed_permission_full(*(guild.value()), *(channel.value()), *(bot_opt.value()), reply_log_data);
    if (!bot_perm.has_value()) {
        co_return;
    }

    //Check the most basic permission
    if (!mln::perms::check_permissions({user_perm.value(), bot_perm.value()}, dpp::permissions::p_manage_roles)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Failed command, either the user or the bot do not have the slashcommand required permission!"), bot());
        co_return;
    }

    const dpp::snowflake user_id = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    const dpp::snowflake role_id = std::get<dpp::snowflake>(event_data.get_parameter("role"));

    //Retrieve command target information
    const std::optional<std::shared_ptr<const dpp::guild_member>> target = co_await mln::caches::get_member_full({guild.value()->id, user_id}, reply_log_data);
    if (!target.has_value()) {
        co_return;
    }

    const dpp::confirmation_callback_t editing_user = co_await bot().co_guild_edit_member(dpp::guild_member{*target.value()}.add_role(role_id));

    if (editing_user.is_error()) {
        const dpp::error_info err = editing_user.get_error();
        const std::string err_text = std::format("Failed to add role [{}] to [{}]!", dpp::role::get_mention(role_id), dpp::user::get_mention(target.value()->user_id));

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, err_text), bot(), &event_data, 
            std::format("{} Error: [{}], details: [{}].", err_text, mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
        std::format("Role [{}] added to [{}]", dpp::role::get_mention(role_id), dpp::user::get_mention(target.value()->user_id))), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed add_emoji command conclusion reply!");
    }
    co_return;
}