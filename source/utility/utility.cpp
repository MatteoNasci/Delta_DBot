#include "utility/utility.h"
#include "utility/constants.h"

#include <dpp/cluster.h>


void mln::utility::print(const dpp::cluster& bot, const dpp::message& msg){
    bot.log(dpp::loglevel::ll_debug, "Printing msg...");

    bot.log(dpp::loglevel::ll_debug, "User: " + msg.author.get_mention());
    bot.log(dpp::loglevel::ll_debug, "Channel: " + std::to_string(msg.channel_id));
    bot.log(dpp::loglevel::ll_debug, "Content: " + msg.content);
    bot.log(dpp::loglevel::ll_debug, "Last edit: " + std::to_string(msg.edited));
    bot.log(dpp::loglevel::ll_debug, "Guild: " + std::to_string(msg.guild_id));
    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(msg.id));
    bot.log(dpp::loglevel::ll_debug, "Sent: " + std::to_string(msg.sent));
    bot.log(dpp::loglevel::ll_debug, "Guild: " + std::to_string(msg.webhook_id));

    for (const auto& att : msg.attachments) {
        mln::utility::print(bot, att);
    }

    for (const auto& com : msg.components) {
        mln::utility::print(bot, com);
    }

    for (const auto& f : msg.file_data) {
        mln::utility::print(bot, f);
    }

    mln::utility::print(bot, msg.interaction);

    bot.log(dpp::loglevel::ll_debug, "Print msg over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::component& com) {
    bot.log(dpp::loglevel::ll_debug, "Printing component...");

    bot.log(dpp::loglevel::ll_debug, "Custom id: " + com.custom_id);
    bot.log(dpp::loglevel::ll_debug, "Disabled: " + std::to_string(com.disabled));
    bot.log(dpp::loglevel::ll_debug, "Label: " + com.label);
    bot.log(dpp::loglevel::ll_debug, "Placeholder: " + com.placeholder);
    bot.log(dpp::loglevel::ll_debug, "Type: " + std::to_string(com.type));
    bot.log(dpp::loglevel::ll_debug, "Value: " + std::holds_alternative<std::monostate>(com.value) ? "Monostate" : (std::holds_alternative<std::string>(com.value) ? 
        std::get<std::string>(com.value) : (std::holds_alternative<std::int64_t>(com.value) ? 
            std::to_string(std::get<int64_t>(com.value)) : std::to_string(std::get<double>(com.value)))));
    bot.log(dpp::loglevel::ll_debug, "Url: " + com.url);

    bot.log(dpp::loglevel::ll_debug, "Print component over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::attachment& att) {
    bot.log(dpp::loglevel::ll_debug, "Printing attachment...");

    bot.log(dpp::loglevel::ll_debug, "Name: " + att.filename);
    bot.log(dpp::loglevel::ll_debug, "Url: " + att.url);
    bot.log(dpp::loglevel::ll_debug, "Proxy url: " + att.proxy_url);
    bot.log(dpp::loglevel::ll_debug, "Type: " + att.content_type);
    bot.log(dpp::loglevel::ll_debug, "Desc: " + att.description);
    bot.log(dpp::loglevel::ll_debug, "Ephemeral: " + std::to_string(att.ephemeral));
    bot.log(dpp::loglevel::ll_debug, "Flags: " + std::to_string(att.flags));
    bot.log(dpp::loglevel::ll_debug, "Expire time: " + std::to_string(att.get_expire_time()));
    bot.log(dpp::loglevel::ll_debug, "Issued time: " + std::to_string(att.get_issued_time()));
    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(att.id));
    bot.log(dpp::loglevel::ll_debug, "Null owner: " + std::to_string((att.owner != nullptr)));
    bot.log(dpp::loglevel::ll_debug, "hws: " + std::to_string(att.height) + ", " + std::to_string(att.width) + ", " + std::to_string(att.size));

    bot.log(dpp::loglevel::ll_debug, "Print attachment over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::message_file_data& msg_fd) {
    bot.log(dpp::loglevel::ll_debug, "Printing message_file_data...");

    bot.log(dpp::loglevel::ll_debug, "Name: " + msg_fd.name);
    bot.log(dpp::loglevel::ll_debug, "Type: " + msg_fd.mimetype);
    //bot.log(dpp::loglevel::ll_debug, "Body: " + msg_fd.content);

    bot.log(dpp::loglevel::ll_debug, "Print message_file_data over.");
}
void mln::utility::print(const dpp::cluster& bot, const dpp::message::message_interaction_struct& msg_i) {
    bot.log(dpp::loglevel::ll_debug, "Printing msg interaction...");

    bot.log(dpp::loglevel::ll_debug, "Id: " + std::to_string(msg_i.id));
    bot.log(dpp::loglevel::ll_debug, "Name: " + msg_i.name);
    bot.log(dpp::loglevel::ll_debug, "Type: " + std::to_string(msg_i.type));
    bot.log(dpp::loglevel::ll_debug, "User id: " + std::to_string(msg_i.usr.id));

    bot.log(dpp::loglevel::ll_debug, "Print msg interaction over.");
}

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
