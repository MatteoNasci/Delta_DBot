#include "utility/utility.h"
#include "utility/constants.h"

#include <dpp/cluster.h>

dpp::task<std::optional<dpp::guild_member>> mln::utility::resolve_guild_member(const dpp::interaction_create_t& event){
    const dpp::command_value& user_param = event.get_parameter("user");
    //If parameter has an user use that, otherwise get sender user
    const dpp::snowflake user_id = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : event.command.usr.id;

    // If we have the guild member in the command's resolved data, return it
    const auto& member_map = event.command.resolved.members;
    if (auto member = member_map.find(user_id); member != member_map.end()) {
        co_return member->second;
    }

    // Try looking in guild cache
    dpp::guild* guild = dpp::find_guild(event.command.guild_id);
    if (guild) {
        // Look in guild's member cache
        if (auto member = guild->members.find(user_id); member != guild->members.end()) {
            co_return member->second;
        }
    }

    // Finally if everything else failed, request API
    dpp::confirmation_callback_t confirmation = co_await event.from->creator->co_guild_get_member(event.command.guild_id, user_id);
    if (confirmation.is_error()) {
        co_return std::nullopt; // Member not found, return empty
    }else {
        co_return confirmation.get<dpp::guild_member>();
    }
}

dpp::task<void> mln::utility::send_msg_recursively(dpp::cluster& bot, const dpp::interaction_create_t& event, const std::string& in_msg, const dpp::snowflake& target_user, bool use_first_reply, bool broadcast){
    size_t text_length = in_msg.length();
    size_t start = 0;
    dpp::async<dpp::confirmation_callback_t> waiting;
    bool waiting_valid = false;
    while (text_length > 0) {
        size_t count_to_send = std::min(mln::constants::get_max_characters_reply_msg(), text_length);

        dpp::message msg{ in_msg.substr(start, count_to_send)};
        if (!broadcast) {
            msg.set_flags(dpp::m_ephemeral);
        }

        if (use_first_reply) {
            use_first_reply = !use_first_reply;
            event.edit_response(msg.set_type(dpp::message_type::mt_reply));
        }else {
            msg.set_type(dpp::message_type::mt_default);
            if (waiting_valid) {
                co_await waiting;
            }
            waiting = broadcast ? bot.co_message_create(msg.set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id)) :
                bot.co_direct_message_create(target_user, msg);
            waiting_valid = true;
        }

        text_length -= count_to_send;
        start += count_to_send;
    }

    if (waiting_valid) {
        co_await waiting;
    }
}
