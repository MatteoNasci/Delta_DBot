#include "commands/slash/avatar.h"
#include "commands/slash/base_slashcommand.h"
#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/guild.h>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

mln::avatar::avatar(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("avatar"), "Get your or another user's avatar image", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from", true))) } {}

dpp::job mln::avatar::command(dpp::slashcommand_t event_data) const {
    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed avatar, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_think(lite_data, true, false, {});

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(lite_data, "Failed avatar retrieval, discord error!", true, "Failed avatar retrieval, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    const dpp::command_value& user_param = event_data.get_parameter("user");
    const dpp::snowflake user_id = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ 0 };

    if (user_id == 0) {
        static const std::string s_err_text = "Failed avatar command, either the user id parameter is invalid or it was not possible to retrieve it.";
        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

        co_return;
    }

    //Retrieve guild data
    const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_task(lite_data.guild_id, lite_data);
    if (!guild.has_value()) {
        co_return;
    }

    //Retrieve member
    const std::optional<std::shared_ptr<const dpp::guild_member>> member = co_await mln::caches::get_member_task(guild.value()->id, user_id, lite_data, &event_data.command.member, &event_data.command.resolved.members);
    if (!member.has_value()) {
        co_return;
    }

    std::string avatar_url = member.value()->get_avatar_url();
    if (avatar_url.empty()) {
        //Member does not have a custom avatar for this server, get their user avatar
        //Retrieve user
        const std::optional<std::shared_ptr<const dpp::user>> user = co_await mln::caches::get_user_task(user_id, lite_data, &event_data.command.usr, &event_data.command.resolved.users);
        if (!user.has_value()) {
            co_return;
        }

        avatar_url = user.value()->get_avatar_url();
    }

    co_await mln::response::co_respond(lite_data, avatar_url, false, "Failed avatar command conclusion reply!");
}

std::optional<std::function<void()>> mln::avatar::job(dpp::slashcommand_t event_data) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::avatar::use_job() const
{
    return false;
}
