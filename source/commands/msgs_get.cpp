#include "commands/msgs_get.h"
#include <variant>

static constexpr int64_t s_msgs_get_min_val{1};
static constexpr int64_t s_msgs_get_max_val{100};

dpp::task<void> msgs_get::command(bot_delta_data_t& data, const dpp::slashcommand_t& event)
{
    int64_t limit = std::get<int64_t>(event.get_parameter("quantity"));
    
    /* get messages using ID of the channel the command was issued in */
    auto msg_get_res = co_await data.bot.co_messages_get(event.command.channel_id, 0, 0, 0, limit);

    if (msg_get_res.is_error()) { /* catching an error to log it */
        std::cout << msg_get_res.get_error().message << std::endl;
        co_return;
    }
    auto messages = msg_get_res.get<dpp::message_map>();
    /* std::get<dpp::message_map>(callback.value) would give the same result */
    std::string contents;
    for (const auto& x : messages) { /* here we iterate through the dpp::message_map we got from callback... */
        contents += x.second.content + '\n'; /* ...where x.first is ID of the current message and x.second is the message itself. */
    }
    event.reply(contents); /* we will see all those messages we got, united as one! */
}
dpp::slashcommand msgs_get::get_command(dpp::cluster& bot)
{
    return dpp::slashcommand(msgs_get::get_command_name(), "Get messages", bot.me.id)
                    .add_option(dpp::command_option(dpp::co_integer, "quantity", "Quantity of messages to get. Max - 100.")
                        .set_min_value(s_msgs_get_min_val).set_max_value(s_msgs_get_max_val));
}
std::string msgs_get::get_command_name(){
    return "msgs_get";
}