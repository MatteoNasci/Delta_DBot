#include "utility/utility.h"
#include "utility/constants.h"

#include <dpp/colors.h>
#include <dpp/channel.h>
#include <dpp/cache.h>

#include <regex>

bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& out_name);

void mln::utility::any_results_da_callback(void* d, int, mln::db_column_data_t&&) {
    bool* const bool_ptr = static_cast<bool*>(d);
    *bool_ptr = true;
}
bool mln::utility::any_results_td_callback(void*, int) {
    return false;
}
const mln::database_callbacks_t& mln::utility::get_any_results_callback() {
    static const mln::database_callbacks_t callbacks{nullptr, mln::utility::any_results_da_callback, mln::utility::any_results_td_callback, nullptr, nullptr, nullptr};
    
    return callbacks;
}
mln::database_callbacks_t mln::utility::get_any_results_callback(bool* found) {
    mln::database_callbacks_t copy{get_any_results_callback()};
    copy.callback_data = static_cast<void*>(found);
    
    return copy;
}

dpp::task<std::tuple<dpp::channel*, dpp::channel>> mln::utility::get_channel(const dpp::interaction_create_t& event_data, const dpp::snowflake& channel_id, dpp::cluster& bot) {
    if (event_data.command.id == 0) {
        co_return{nullptr, {}};
    }

    auto it = event_data.command.resolved.channels.find(channel_id);
    if (it != event_data.command.resolved.channels.end()) {
        dpp::channel channel = it->second;
        if (channel.id != 0) {
            co_return{nullptr, std::move(channel)};
        }
    }

    co_return co_await mln::utility::get_channel(channel_id, bot);
}
dpp::task<std::tuple<dpp::channel*, dpp::channel>> mln::utility::get_channel(const dpp::snowflake& channel_id, dpp::cluster& bot) {
    if (channel_id == 0) {
        co_return {nullptr, {}};
    }

    dpp::channel* channel = dpp::find_channel(channel_id);
    if (channel != nullptr && channel->id != 0) {
        co_return {channel, {}};
    }

    const dpp::confirmation_callback_t result = co_await bot.co_channel_get(channel_id);
    if (result.is_error()) {
        bot.log(dpp::loglevel::ll_warning, "Failed to retrieve channel, even from API! Error: " + result.get_error().human_readable);
        co_return {nullptr, {}};
    }
    
    co_return {nullptr, {std::move(result.get<dpp::channel>())}};
}
dpp::task<std::tuple<dpp::guild*, dpp::guild>> mln::utility::get_guild(const dpp::interaction_create_t& event_data, dpp::cluster& bot) {
    if (event_data.command.id == 0) {
        co_return {nullptr, {}};
    }

    co_return co_await mln::utility::get_guild(event_data.command.guild_id, bot);
}
dpp::task<std::tuple<dpp::guild*, dpp::guild>> mln::utility::get_guild(const dpp::snowflake& guild_id, dpp::cluster& bot) {
    if (guild_id == 0) {
        co_return {nullptr, {}};
    }

    dpp::guild* guild = dpp::find_guild(guild_id);
    if (guild != nullptr && guild->id != 0) {
        co_return {guild, {}};
    }
    
    const dpp::confirmation_callback_t result = co_await bot.co_guild_get(guild_id);
    if (result.is_error()) {
        bot.log(dpp::loglevel::ll_warning, "Failed to retrieve guild, even from API! Error: " + result.get_error().human_readable);
        co_return{nullptr, {}};
    }

    co_return {nullptr, {std::move(result.get<dpp::guild>())}};
}
dpp::task<std::optional<dpp::guild_member>> mln::utility::get_member(const dpp::interaction_create_t& event_data, const dpp::guild* const guild, const dpp::snowflake& user_id, dpp::cluster& bot) {
    if (event_data.command.id == 0 || user_id == 0) {
        co_return std::nullopt;
    }

    const auto it = event_data.command.resolved.members.find(user_id);
    if (it != event_data.command.resolved.members.end()) {
        co_return it->second;
    }

    co_return co_await mln::utility::get_member(guild, user_id, bot);
}
dpp::task<std::optional<dpp::guild_member>> mln::utility::get_member(const dpp::interaction_create_t& event_data, const dpp::snowflake& user_id, dpp::cluster& bot) {
    if (event_data.command.id == 0 || user_id == 0) {
        co_return std::nullopt;
    }

    const auto it = event_data.command.resolved.members.find(user_id);
    if (it != event_data.command.resolved.members.end()) {
        co_return it->second;
    }

    std::tuple<dpp::guild*, dpp::guild> guild_pair = co_await mln::utility::get_guild(event_data, bot);
    dpp::guild* ptr = std::get<0>(guild_pair);
    if (ptr == nullptr) {
        ptr = &(std::get<1>(guild_pair));
    }

    co_return co_await mln::utility::get_member(ptr, user_id, bot);
}
dpp::task<std::optional<dpp::guild_member>> mln::utility::get_member(const dpp::guild* const guild, const dpp::snowflake& user_id, dpp::cluster& bot) {
    if (guild == nullptr || guild->id == 0 || user_id == 0) {
        co_return std::nullopt;
    }

    const auto it_usr = guild->members.find(user_id);
    if (it_usr != guild->members.end()) {

        dpp::guild_member user = it_usr->second;
        if (user.user_id != 0) {
            co_return std::move(user);
        }
    }

    const dpp::confirmation_callback_t result = co_await bot.co_guild_get_member(guild->id, user_id);
    
    if (result.is_error()) {
        bot.log(dpp::loglevel::ll_warning, "Failed to retrieve guild member, even from API! Error: " + result.get_error().human_readable);
        co_return std::nullopt;
    }

    co_return std::move(result.get<dpp::guild_member>());
}

dpp::task<std::tuple<std::optional<dpp::guild_member>, std::optional<dpp::guild_member>>> mln::utility::get_members(const dpp::interaction_create_t& event_data, dpp::cluster& bot) {
    if (event_data.command.id == 0) {
        co_return {std::nullopt, std::nullopt};
    }

    std::tuple<dpp::guild*, dpp::guild> guild = co_await mln::utility::get_guild(event_data, bot);

    dpp::guild* ptr = std::get<0>(guild);
    if (ptr == nullptr) {
        ptr = &std::get<1>(guild);
    }

    co_return {co_await mln::utility::get_member(ptr, event_data.command.usr.id, bot), 
        co_await mln::utility::get_member(ptr, event_data.command.application_id, bot)};
}

bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member*>& users, const std::vector<dpp::permissions>& permissions_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const dpp::guild_member* member : users) {
        if (member == nullptr || member->user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(*member, *channel), permissions_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member*>& users, dpp::permissions permission_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const dpp::guild_member* member : users) {
        if (member == nullptr || member->user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(*member, *channel), permission_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member* user, const std::vector<dpp::permissions>& permissions_to_check) {
    if (guild == nullptr || channel == nullptr || user == nullptr || user->user_id == 0) {
        return false;
    }

    return mln::utility::check_permissions(guild->permission_overwrites(*user, *channel), permissions_to_check);
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member* user, dpp::permissions permission_to_check) {
    if (guild == nullptr || channel == nullptr || user == nullptr || user->user_id == 0) {
        return false;
    }

    return mln::utility::check_permissions(guild->permission_overwrites(*user, *channel), permission_to_check);
}

bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member>& users, const std::vector<dpp::permissions>& permissions_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const dpp::guild_member& member : users) {
        if (member.user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(member, *channel), permissions_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member>& users, dpp::permissions permission_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const dpp::guild_member& member : users) {
        if (member.user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(member, *channel), permission_to_check)) {
            return false;
        }
    }
    
    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member& user, const std::vector<dpp::permissions>& permissions_to_check) {
    if (guild == nullptr || channel == nullptr || user.user_id == 0) {
        return false;
    }

    return mln::utility::check_permissions(guild->permission_overwrites(user, *channel), permissions_to_check);
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member& user, dpp::permissions permission_to_check) {
    if (guild == nullptr || channel == nullptr || user.user_id == 0) {
        return false;
    }

    return mln::utility::check_permissions(guild->permission_overwrites(user, *channel), permission_to_check);
}

bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<std::optional<dpp::guild_member>>& users, const std::vector<dpp::permissions>& permissions_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const std::optional<dpp::guild_member>& opt_member : users) {
        if (!opt_member.has_value() || opt_member->user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(*opt_member, *channel), permissions_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<std::optional<dpp::guild_member>>& users, dpp::permissions permission_to_check) {
    if (guild == nullptr || channel == nullptr || guild->id == 0 || channel->id == 0) {
        return false;
    }

    for (const std::optional<dpp::guild_member>& opt_member : users) {
        if (!opt_member.has_value() || opt_member->user_id == 0 || !mln::utility::check_permissions(guild->permission_overwrites(*opt_member, *channel), permission_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::optional<dpp::guild_member>& user, const std::vector<dpp::permissions>& permissions_to_check) {
    if (!user.has_value()) {
        return false;
    }

    return mln::utility::check_permissions(guild, channel, *user, permissions_to_check);
}
bool mln::utility::check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::optional<dpp::guild_member>& user, dpp::permissions permission_to_check) {
    if (!user.has_value()) {
        return false;
    }

    return mln::utility::check_permissions(guild, channel, *user, permission_to_check);
}


bool mln::utility::check_permissions(const std::vector<dpp::permission>& permissions, const std::vector<dpp::permissions>& permissions_to_check) {

    for (const dpp::permissions& flag_to_check : permissions_to_check) {
        for (const dpp::permission& perm : permissions) {
            if (!perm.can(flag_to_check)) {
                return false;
            }
        }
    }

    return true;
}
bool mln::utility::check_permissions(const std::vector<dpp::permission>& permissions, dpp::permissions permission_to_check) {
    for (const dpp::permission& perm : permissions) {
        if (!perm.can(permission_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(dpp::permission permission, const std::vector<dpp::permissions>& permissions_to_check) {
    for (const dpp::permissions& flag_to_check : permissions_to_check) {
        if (!permission.can(flag_to_check)) {
            return false;
        }
    }

    return true;
}
bool mln::utility::check_permissions(dpp::permission permission, dpp::permissions permission_to_check) {
    return permission.can(permission_to_check);
}

dpp::task<void> mln::utility::co_conclude_thinking_response(dpp::async<dpp::confirmation_callback_t>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always) {
    dpp::confirmation_callback_t confirmation = co_await thinking;
    bool logged = false;
    if (confirmation.is_error()) {
        bot.log(dpp::loglevel::ll_error, to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ". Also failed thinking confirmation : " + confirmation.get_error().human_readable);
        logged = true;
    }

    confirmation = co_await event_data.co_edit_response(to_respond);
    if (confirmation.is_error()) {
        bot.log(dpp::loglevel::ll_error, to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ". Also failed edit response confirmation : " + confirmation.get_error().human_readable);
        logged = true;
    }

    if (std::get<0>(log_always) && !logged) {
        bot.log(std::get<1>(log_always), to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ".");
    }
}
dpp::task<void> mln::utility::co_conclude_thinking_response(std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always) {
    bool logged = false;
    if (thinking.has_value()) {
        dpp::confirmation_callback_t confirmation = co_await thinking.value();
        if (confirmation.is_error()) {
            bot.log(dpp::loglevel::ll_error, to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ". Also failed thinking confirmation : " + confirmation.get_error().human_readable);
            logged = true;
        }
    }

    dpp::confirmation_callback_t confirmation = co_await event_data.co_edit_response(to_respond);
    if (confirmation.is_error()) {
        bot.log(dpp::loglevel::ll_error, to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ". Also failed edit response confirmation : " + confirmation.get_error().human_readable);
        logged = true;
    }

    if (std::get<0>(log_always) && !logged) {
        bot.log(std::get<1>(log_always), to_respond + " " + event_data.command.get_command_name() + ", from usr: " + std::to_string(event_data.command.usr.id) + " in guild " + std::to_string(event_data.command.guild_id) + " in channel " + std::to_string(event_data.command.channel_id) + ".");
    }
}

bool mln::utility::extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id) {
    static const std::regex url_regex(R"(https://discord.com/channels/(\d+)/(\d+)/(\d+))");
    std::smatch match;

    const bool regex_result = std::regex_search(msg_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 4;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
        out_message_id = std::stoull(match[3].str());
    }

    return valid_results;
}
bool mln::utility::extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    bool result = mln::utility::extract_attachment_url_data(attachment_url, out_guild_id, out_channel_id);
    if (!result) {
        result = mln::utility::extract_ephemeral_attachment_url_data(attachment_url, out_guild_id, out_channel_id);
    }
    return result;
}
bool mln::utility::extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    bool result = mln::utility::extract_attachment_url_data(attachment_url, out_guild_id, out_channel_id, name);
    if (!result) {
        result = mln::utility::extract_ephemeral_attachment_url_data(attachment_url, out_guild_id, out_channel_id, name);
    }
    return result;
}
bool mln::utility::extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/attachments/(\d+)/(\d+)/)");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id);
}
bool mln::utility::extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/attachments/(\d+)/(\d+)/([^\/?]+))");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id, name);
}
bool mln::utility::extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/ephemeral-attachments/(\d+)/(\d+)/)");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id);
}
bool mln::utility::extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name) {
    static const std::regex url_regex(R"(https://cdn.discordapp.com/ephemeral-attachments/(\d+)/(\d+)/([^\/?]+))");
    return extract_attachment_url(url_regex, attachment_url, out_guild_id, out_channel_id, name);
}
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id) {
    std::smatch match;

    const bool regex_result = std::regex_search(attachment_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 3;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
    }

    return valid_results;
}
bool extract_attachment_url(const std::regex& url_regex, const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& out_name) {
    std::smatch match;

    const bool regex_result = std::regex_search(attachment_url, match, url_regex);
    const bool valid_results = regex_result && match.size() == 4;
    if (valid_results) {
        out_guild_id = std::stoull(match[1].str());
        out_channel_id = std::stoull(match[2].str());
        out_name = std::move(match[3].str());
    }

    return valid_results;
}
dpp::job mln::utility::manage_paginated_embed(dpp::interaction_create_t event_data, dpp::cluster* bot, const std::shared_ptr<std::vector<std::string>> text_ptr, uint64_t time_limit_seconds) {
    event_data.edit_response("Processing the data, please wait...");

    std::vector<dpp::message> messages{};
    //There's a limit of 10 embeds per message, if more are needed just use several msgs I guess
    //Prepare all the embeds beforehand, then use the when_any stuff for the reaction pagine behavior.
    //I'm gonna need embeds count

    co_return;
}