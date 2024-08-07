#include "commands/avatar.h"
#include "utility/utility.h"

#include <dpp/coro/async.h>
#include <dpp/snowflake.h>
#include <dpp/restresults.h>
#include <dpp/guild.h>

#include <variant>

static constexpr uint16_t s_avatar_pixel_size{ 512 };

dpp::task<void> mln::avatar::command(mln::bot_delta_data_t &data, const dpp::slashcommand_t &event){
    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;

    // Send a "<bot> is thinking..." message, to wait on later so we can edit
    dpp::async thinking = event.co_thinking(!broadcast);
    // Call our coroutine defined above to retrieve the member requested
    std::optional<dpp::guild_member> member = co_await mln::utility::resolve_guild_member(event);
    if (!member.has_value()) {
        // Wait for the thinking response to arrive to make sure we can edit
        dpp::message msg{ "User not found in this server!" };
        if(!broadcast) {
            msg.set_flags(dpp::m_ephemeral);
        }
        co_await thinking;
        event.edit_original_response(msg);
        co_return;
    }
 
    std::string avatar_url = member->get_avatar_url(s_avatar_pixel_size);
    if (avatar_url.empty()) { // Member does not have a custom avatar for this server, get their user avatar
        dpp::confirmation_callback_t confirmation = co_await event.from->creator->co_user_get_cached(member->user_id);
        if (confirmation.is_error()) {
            // Wait for the thinking response to arrive to make sure we can edit
            dpp::message msg{ "User not found!" };
            if (!broadcast) {
                msg.set_flags(dpp::m_ephemeral);
            }
            co_await thinking;
            event.edit_original_response(msg);
            co_return;
        }
        avatar_url = confirmation.get<dpp::user_identified>().get_avatar_url(s_avatar_pixel_size);
    }
 
    // Wait for the thinking response to arrive to make sure we can edit
    dpp::message msg{ avatar_url };
    if (!broadcast) {
        msg.set_flags(dpp::m_ephemeral);
    }
    co_await thinking;
    event.edit_original_response(msg);
}

dpp::slashcommand mln::avatar::get_command(dpp::cluster &bot){
    return dpp::slashcommand(mln::avatar::get_command_name(), "Get your or another user's avatar image", bot.me.id)
                .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from"))
                .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel"));
}

std::string mln::avatar::get_command_name(){
    return "avatar";
}
