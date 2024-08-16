#include "commands/slash/avatar.h"
#include "bot_delta.h"
#include "utility/utility.h"

static constexpr uint16_t s_avatar_pixel_size{ 512 };

mln::avatar::avatar(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("avatar", "Get your or another user's avatar image", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_user, "user", "User to fetch the avatar from", true))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel", false)))) {}

dpp::task<void> mln::avatar::command(const dpp::slashcommand_t& event_data){
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;

    // Send a "<bot> is thinking..." message, to wait on later so we can edit
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    const dpp::command_value& user_param = event_data.get_parameter("user");
    //If parameter has an user use that, otherwise get sender user
    const dpp::snowflake user_id = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : event_data.command.usr.id;
    // Call our coroutine to retrieve the member requested
    const std::optional<dpp::guild_member> member = co_await mln::utility::resolve_guild_member(event_data, user_id);

    if (!member.has_value()) {
        co_await thinking;
        event_data.edit_response("User not found in this server!");
        co_return;
    }
    
    std::string avatar_url = member->get_avatar_url(s_avatar_pixel_size);
    if (avatar_url.empty()) { // Member does not have a custom avatar for this server, get their user avatar
        const dpp::confirmation_callback_t confirmation = co_await event_data.from->creator->co_user_get_cached(member->user_id);
        if (confirmation.is_error()) {
            // Wait for the thinking response to arrive to make sure we can edit
            co_await thinking;
            event_data.edit_response("User's avatar not found! Error: " + confirmation.get_error().human_readable);
            co_return;
        }
        avatar_url = confirmation.get<dpp::user_identified>().get_avatar_url(s_avatar_pixel_size);
    }
 
    co_await thinking;
    event_data.edit_response(avatar_url);
}