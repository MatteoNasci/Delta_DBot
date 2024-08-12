#include "commands/slash/msgs_get.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::msgs_get::msgs_get(bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("get_messages", "Get messages", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_integer, "quantity", "Quantity of messages to get. Max - " + std::to_string(static_cast<int64_t>(mln::constants::get_max_retrievable_msgs())) + ".", true)
            .set_min_value(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_retrievable_msgs())))
            .set_max_value(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_retrievable_msgs()))))
        .add_option(dpp::command_option(dpp::co_user, "user", "User to retrieve msgs from.", true))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "If true it will broadcast the msgs, otherwise only the user will see them. Default: false.", false)))) {}

dpp::job mln::msgs_get::command(dpp::slashcommand_t event){
    //TODO problems with sending bot msgs to pm (when they cannot be representend by the original reply). Everything else works (bot msgs to channel, user msgs to pm or channel).
    const int64_t limit = std::get<int64_t>(event.get_parameter("quantity"));
    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    
    auto waiting_res = event.co_thinking(!broadcast);
    /* get messages using ID of the channel the command was issued in */
    auto msg_get_res = co_await delta()->bot.co_messages_get(event.command.channel_id, 0, 0, 0, limit);

    const dpp::snowflake user = std::get<dpp::snowflake>(event.get_parameter("user"));

    if (msg_get_res.is_error()) {
        delta()->bot.log(dpp::loglevel::ll_debug, msg_get_res.get_error().message);
        dpp::message msg_failed("Failed to retrieve messages: " + msg_get_res.get_error().message);
        if (!broadcast) {
             msg_failed.set_flags(dpp::m_ephemeral);
        }

        co_await waiting_res;
        event.reply(msg_failed);
        co_return;
    }

    auto messages = msg_get_res.get<dpp::message_map>();
    //Ordering unsorted map to show messages in order
    std::vector<dpp::snowflake> keys(messages.size());
    size_t current_index = 0;
    for (const auto& it : messages) {
        keys[current_index++] = it.first;
    }
    std::sort(keys.begin(), keys.end(), [](const dpp::snowflake& a, const dpp::snowflake& b) { return a.get_creation_time() < b.get_creation_time(); });

    bool first_reply = true;
    std::string contents;
    for (size_t i = 0; i < keys.size(); ++i) {
        const dpp::message& current_msg = messages.at(keys[i]);
        const size_t current_length = contents.length();
        const size_t to_add_length = current_msg.content.length();
        
        if (to_add_length == 0) {
            continue;
        }
        if (current_msg.author.id != user) {
            continue;
        }

        if (to_add_length > mln::constants::get_max_characters_reply_msg()) {
            dpp::message msg_error("One or more of the retrieved messagges has a length that exceeds the limit the bot can handle! Msg length: " 
                + std::to_string(to_add_length) + ", max allowed: " + std::to_string(mln::constants::get_max_characters_reply_msg()));

            if (!broadcast) {
                msg_error.set_flags(dpp::m_ephemeral);
            }

            co_await waiting_res;
            if (first_reply) {
                event.edit_response(msg_error);
                first_reply = !first_reply;
            }
            else {
                broadcast ? delta()->bot.message_create(msg_error.set_guild_id(event.command.guild_id).set_channel_id(event.command.channel_id)) :
                    delta()->bot.direct_message_create(user, msg_error);
            }
            co_return;
        }

        if (current_length + to_add_length > mln::constants::get_max_characters_reply_msg()) {
            dpp::message msg_partial;
            if (!broadcast) {
                msg_partial.set_flags(dpp::m_ephemeral);
            }

            if (first_reply) {
                msg_partial.set_content("Messages from " + dpp::user::get_mention(user) + " retrieved!" + (broadcast ? "" : " Msgs exceed the comment character limits, sent as pm instead!"));
                co_await waiting_res;
                waiting_res = event.co_edit_response(msg_partial);
                first_reply = !first_reply;
            }

            msg_partial.set_content(contents);
            co_await waiting_res;
            waiting_res = broadcast ? delta()->bot.co_message_create(msg_partial.set_channel_id(event.command.channel_id).set_guild_id(event.command.guild_id)) :
                delta()->bot.co_direct_message_create(user, msg_partial);
            

            contents = "";
        }

        const size_t final_length = contents.length();
        const bool add_new_line = final_length != 0 && final_length + to_add_length < mln::constants::get_max_characters_reply_msg();
        contents += (add_new_line ? "\n" : "") + current_msg.content;
    }

    dpp::message msgs_retrieved;
    if (!broadcast) {
        msgs_retrieved.set_flags(dpp::m_ephemeral);
    }
    co_await waiting_res;
    if (contents.length() == 0) {
        if (first_reply) {
            event.edit_response(msgs_retrieved.set_content("No replies found from user " + dpp::user::get_mention(user)));
            first_reply = !first_reply;
        }
        co_return;
    }

    msgs_retrieved.set_content(contents);

    if (first_reply) {
        event.edit_response(msgs_retrieved);
        first_reply = !first_reply;
    }
    else {
        broadcast ? delta()->bot.message_create(msgs_retrieved.set_guild_id(event.command.guild_id).set_channel_id(event.command.channel_id)) :
            delta()->bot.direct_message_create(user, msgs_retrieved);
    }
}
