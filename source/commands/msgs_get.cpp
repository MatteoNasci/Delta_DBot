#include "commands/msgs_get.h"
#include "utility/constants.h"

#include <variant>

static constexpr int64_t s_msgs_get_min_val{1};
static constexpr int64_t s_msgs_get_max_val{1000};

dpp::task<void> mln::msgs_get::command(mln::bot_delta_data_t& data, const dpp::slashcommand_t& event){
    const int64_t limit = std::get<int64_t>(event.get_parameter("quantity"));
    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    
    auto waiting_res = event.co_thinking(!broadcast);
    /* get messages using ID of the channel the command was issued in */
    auto msg_get_res = co_await data.bot.co_messages_get(event.command.channel_id, 0, 0, 0, limit);

    const dpp::snowflake user = std::get<dpp::snowflake>(event.get_parameter("user"));

    if (msg_get_res.is_error()) {
        data.bot.log(dpp::loglevel::ll_debug, msg_get_res.get_error().message);
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
                msg_error.set_channel_id(event.command.channel_id);
                msg_error.set_guild_id(event.command.guild_id);
                broadcast ? data.bot.message_create(msg_error) : data.bot.direct_message_create(user, msg_error);
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
            msg_partial.set_channel_id(event.command.channel_id);
            msg_partial.set_guild_id(event.command.guild_id);
            co_await waiting_res;
            waiting_res = broadcast ? data.bot.co_message_create(msg_partial) : data.bot.co_direct_message_create(user, msg_partial);
            

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
        msgs_retrieved.set_channel_id(event.command.channel_id);
        msgs_retrieved.set_guild_id(event.command.guild_id);
        broadcast ? data.bot.message_create(msgs_retrieved) : data.bot.direct_message_create(user, msgs_retrieved);
    }
}
dpp::slashcommand mln::msgs_get::get_command(dpp::cluster& bot){
    return dpp::slashcommand(mln::msgs_get::get_command_name(), "Get messages", bot.me.id)
        .add_option(dpp::command_option(dpp::co_integer, "quantity", "Quantity of messages to get. Max - " + std::to_string(s_msgs_get_max_val) + ".", true)
            .set_min_value(s_msgs_get_min_val).set_max_value(s_msgs_get_max_val))
        .add_option(dpp::command_option(dpp::co_user, "user", "User to retrieve msgs from.", true))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "If true it will broadcast the msgs, otherwise only the user will see them. Default: false.", false));
}
std::string mln::msgs_get::get_command_name(){
    return "msgs_get";
}