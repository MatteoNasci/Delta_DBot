#include "commands/msgs_get.h"
#include <variant>

static constexpr int64_t s_msgs_get_min_val{1};
static constexpr int64_t s_msgs_get_max_val{100};

dpp::task<void> msgs_get::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
    const int64_t limit = std::get<int64_t>(event.get_parameter("quantity"));
    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    
    /* get messages using ID of the channel the command was issued in */
    auto msg_get_res = co_await data.bot.co_messages_get(event.command.channel_id, 0, 0, 0, limit);

    if (msg_get_res.is_error()) {
        data.bot.log(dpp::loglevel::ll_debug, msg_get_res.get_error().message);
        co_return;
    }
    auto messages = msg_get_res.get<dpp::message_map>();
    /* std::get<dpp::message_map>(callback.value) would give the same result */
    std::string contents;
    for (const auto& x : messages) {
        contents += x.second.content + '\n';
    }

    dpp::message msgs_retrieved;
    if (contents.size() >= 2000) {
        msgs_retrieved.set_content("Failed to send the messages! The size exceeds the 2000 character limit!");
    }else {
        msgs_retrieved.set_content(contents);
    }

    if (!broadcast) {
        msgs_retrieved = msgs_retrieved.set_flags(dpp::m_ephemeral);
    }
    event.reply(msgs_retrieved);
}
dpp::slashcommand msgs_get::get_command(dpp::cluster& bot){
    return dpp::slashcommand(msgs_get::get_command_name(), "Get messages", bot.me.id)
        .add_option(dpp::command_option(dpp::co_integer, "quantity", "Quantity of messages to get. Max - 100.", true)
            .set_min_value(s_msgs_get_min_val).set_max_value(s_msgs_get_max_val))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "If true it will broadcast the msgs, otherwise only the command user will see them. Default: false.", false));
}
std::string msgs_get::get_command_name(){
    return "msgs_get";
}