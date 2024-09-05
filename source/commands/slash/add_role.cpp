#include "commands/slash/add_role.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"

mln::add_role::add_role(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("add_role", "Give user a role", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to give role to", true))
        .add_option(dpp::command_option(dpp::co_role, "role", "Role to give", true))
    )) {}

dpp::task<void> mln::add_role::command(const dpp::slashcommand_t& event_data){
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    //Retrieve guild data
    std::optional<std::shared_ptr<const dpp::guild>> guild = mln::caches::get_guild(event_data.command.guild_id);
    if (!guild.has_value()) {
        guild = co_await mln::caches::get_guild_task(event_data.command.guild_id);
        if (!guild.has_value()) {
            //Error can't find guild
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve guild data! guild_id: "
                + std::to_string(event_data.command.guild_id));
            co_return;
        }
    }

    //Retrieve channel data
    std::optional<std::shared_ptr<const dpp::channel>> channel = mln::caches::get_channel(event_data.command.channel_id, &event_data);
    if (!channel.has_value()) {
        channel = co_await mln::caches::get_channel_task(event_data.command.channel_id);
        if (!channel.has_value()) {
            //Error can't find channel
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve channel data! channel_id: "
                + std::to_string(event_data.command.channel_id));
            co_return;
        }
    }

    //Retrieve command user and bot information
    std::optional<std::shared_ptr<const dpp::guild_member>> user = mln::caches::get_member({guild.value()->id, event_data.command.usr.id}, &event_data);
    if (!user.has_value()) {
        user = co_await mln::caches::get_member_task({guild.value()->id, event_data.command.usr.id});
        if (!user.has_value()) {
            //Error can't find user
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve command user data! user_id: "
                + std::to_string(event_data.command.usr.id));
            co_return;
        }
    }

    std::optional<std::shared_ptr<const dpp::guild_member>> bot = mln::caches::get_member({guild.value()->id, event_data.command.application_id}, &event_data);
    if (!bot.has_value()) {
        bot = co_await mln::caches::get_member_task({guild.value()->id, event_data.command.application_id});
        if (!bot.has_value()) {
            //Error can't find bot
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve command bot data! bot id: "
                + std::to_string(event_data.command.application_id));
            co_return;
        }
    }

    //Retrieve user and bot perms, then return an error if the user and the bot don't have the required permissions
    std::optional<dpp::permission> user_perm = mln::perms::get_computed_permission(*(guild.value()), *(channel.value()), *(user.value()), &event_data);
    if (!user_perm.has_value()) {
        user_perm = co_await mln::perms::get_computed_permission_task(*(guild.value()), *(channel.value()), *(user.value()), &event_data);
        if (!user_perm.has_value()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve user permission data! user_id: "
                + std::to_string(user.value()->user_id));
            co_return;
        }
    }

    std::optional<dpp::permission> bot_perm = mln::perms::get_computed_permission(*(guild.value()), *(channel.value()), *(bot.value()), &event_data);
    if (!bot_perm.has_value()) {
        bot_perm = co_await mln::perms::get_computed_permission_task(*(guild.value()), *(channel.value()), *(bot.value()), &event_data);
        if (!bot_perm.has_value()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve bot permission data! bot id: "
                + std::to_string(bot.value()->user_id));
            co_return;
        }
    }

    //Check the most basic permission
    if (!mln::perms::check_permissions({user_perm.value(), bot_perm.value()}, dpp::permissions::p_manage_roles)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, either the user or the bot do not have the slashcommand required permission!", {true, dpp::loglevel::ll_debug});
        co_return;
    }

    //TODO check if the user/bot actually can give the selected role to the target (more checks needed probably)

    const dpp::snowflake user_id = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    const dpp::snowflake role_id = std::get<dpp::snowflake>(event_data.get_parameter("role"));

    //Retrieve command target information
    std::optional<std::shared_ptr<const dpp::guild_member>> target = mln::caches::get_member({guild.value()->id, user_id}, &event_data);
    if (!target.has_value()) {
        target = co_await mln::caches::get_member_task({guild.value()->id, event_data.command.usr.id});
        if (!target.has_value()) {
            //Error can't find user
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve command user data! user_id: "
                + std::to_string(user_id));
            co_return;
        }
    }

    const dpp::confirmation_callback_t editing_user = co_await delta()->bot.co_guild_edit_member(dpp::guild_member{*target.value()}.add_role(role_id));

    dpp::message msg{};
    if (editing_user.is_error()) {
        msg.set_content("Failed to add role " + dpp::role::get_mention(role_id) + " to " + dpp::user::get_mention(user_id) +". Error: " + editing_user.get_error().human_readable);
    }else {
        msg.set_content("Role " + dpp::role::get_mention(role_id) + " added to " + dpp::user::get_mention(user_id));
    }

    co_await thinking;
    event_data.edit_response(msg);
}