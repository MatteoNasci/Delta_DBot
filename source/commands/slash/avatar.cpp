#include "commands/slash/avatar.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/caches.h"

mln::avatar::avatar(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("avatar", "Get your or another user's avatar image", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from", true))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel", false)))) {}

dpp::task<void> mln::avatar::command(const dpp::slashcommand_t& event_data){
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;

    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    const dpp::snowflake user_id = std::get<dpp::snowflake>(event_data.get_parameter("user"));

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

    //Retrieve member
    std::optional<std::shared_ptr<const dpp::guild_member>> member = mln::caches::get_member({guild.value()->id, user_id}, &event_data);
    if (!member.has_value()) {
        member = co_await mln::caches::get_member_task({guild.value()->id, user_id});
        if (!member.has_value()) {
            //Error can't find user
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve member data! user_id: "
                + std::to_string(user_id));
            co_return;
        }
    }
    //TODO verify if I need permissions for responding to interaction (like send_msg, attach file)
    //TODO answer: yes I do

    std::string avatar_url = member.value()->get_avatar_url();
    if (avatar_url.empty()) { 
        //Member does not have a custom avatar for this server, get their user avatar
        //Retrieve user
        std::optional<std::shared_ptr<const dpp::user>> user = mln::caches::get_user(user_id, &event_data);
        if (!user.has_value()) {
            user = co_await mln::caches::get_user_task(user_id);
            if (!user.has_value()) {
                //Error can't find user
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve user data! user_id: "
                    + std::to_string(user_id));
                co_return;
            }
        }

        avatar_url = user.value()->get_avatar_url();
    }
 
    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, avatar_url, {false, dpp::loglevel::ll_debug});
}