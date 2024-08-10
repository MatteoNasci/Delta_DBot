#include "utility/utility.h"

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
