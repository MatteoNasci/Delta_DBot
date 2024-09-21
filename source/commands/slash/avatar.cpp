#include "commands/slash/avatar.h"
#include "utility/utility.h"
#include "utility/caches.h"
#include "utility/response.h"
#include "utility/reply_log_data.h"

#include <dpp/dispatcher.h>
#include <dpp/cluster.h>

mln::avatar::avatar(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("avatar", "Get your or another user's avatar image", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from", true))) } {}

dpp::task<void> mln::avatar::command(const dpp::slashcommand_t& event_data) const {
    mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot());

    const dpp::snowflake user_id = std::get<dpp::snowflake>(event_data.get_parameter("user"));

    const reply_log_data_t reply_log_data{ &event_data, &bot(), false };
    //Retrieve guild data
    const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_full(event_data.command.guild_id, reply_log_data);
    if (!guild.has_value()) {
        co_return;
    }

    //Retrieve member
    const std::optional<std::shared_ptr<const dpp::guild_member>> member = co_await mln::caches::get_member_full({guild.value()->id, user_id}, reply_log_data);
    if (!member.has_value()) {
        co_return;
    }

    std::string avatar_url = member.value()->get_avatar_url();
    if (avatar_url.empty()) { 
        //Member does not have a custom avatar for this server, get their user avatar
        //Retrieve user
        const std::optional<std::shared_ptr<const dpp::user>> user = co_await mln::caches::get_user_full(user_id, reply_log_data);
        if (!user.has_value()) {
            co_return;
        }

        avatar_url = user.value()->get_avatar_url();
    }
 
    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, avatar_url), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed avatar command conclusion reply!");
    }
    co_return;
}