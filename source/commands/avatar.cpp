#include "commands/avatar.h"
#include "utility/utility.h"

#include <dpp/coro/async.h>
#include <dpp/snowflake.h>
#include <dpp/restresults.h>
#include <dpp/guild.h>

#include <variant>

static constexpr uint16_t s_avatar_pixel_size{ 512 };

dpp::task<void> avatar::command(bot_delta_data_t &data, const dpp::slashcommand_t &event)
{
    // Send a "<bot> is thinking..." message, to wait on later so we can edit
    dpp::async thinking = event.co_thinking(false);
    // Call our coroutine defined above to retrieve the member requested
    std::optional<dpp::guild_member> member = co_await utility::resolve_guild_member(event);
    if (!member.has_value()) {
        // Wait for the thinking response to arrive to make sure we can edit
        co_await thinking;
        event.edit_original_response(dpp::message{"User not found in this server!"});
        co_return;
    }
 
    std::string avatar_url = member->get_avatar_url(s_avatar_pixel_size);
    if (avatar_url.empty()) { // Member does not have a custom avatar for this server, get their user avatar
        dpp::confirmation_callback_t confirmation = co_await event.from->creator->co_user_get_cached(member->user_id);
        if (confirmation.is_error()) {
            // Wait for the thinking response to arrive to make sure we can edit
            co_await thinking;
            event.edit_original_response(dpp::message{"User not found!"});
            co_return;
        }
        avatar_url = confirmation.get<dpp::user_identified>().get_avatar_url(s_avatar_pixel_size);
    }
 
    // Wait for the thinking response to arrive to make sure we can edit
    co_await thinking;
    event.edit_original_response(dpp::message{avatar_url});
}

dpp::slashcommand avatar::get_command(dpp::cluster &bot){
    return dpp::slashcommand(avatar::get_command_name(), "Get your or another user's avatar image", bot.me.id)
                .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from"));
}

std::string avatar::get_command_name(){
    return "avatar";
}
