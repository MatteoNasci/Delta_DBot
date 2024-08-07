#include "commands/add_emoji.h"

#include <dpp/snowflake.h>
#include <dpp/message.h>
#include <dpp/coro.h>

#include <variant>
#include <unordered_map>

dpp::task<void> mln::add_emoji::command(mln::bot_delta_data_t &data, const dpp::slashcommand_t &event){
    dpp::cluster *cluster = event.from->creator;
    
    dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
    std::string emoji_name = std::get<std::string>(event.get_parameter("name"));
    
    const dpp::attachment &attachment = event.command.get_resolved_attachment(file_id);
    const std::string attachment_content_type = attachment.content_type;
    const std::string attachment_url = attachment.url;

    const dpp::command_value broadcast_param = event.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;

    static const std::unordered_map<std::string, dpp::image_type> allowed_image_types{
        {"image/png", dpp::image_type::i_png},
        {"image/gif", dpp::image_type::i_gif},
        {"image/jpeg", dpp::image_type::i_jpg},
        {"image/webp", dpp::image_type::i_webp}
    };

    auto allowed_img_type_it = allowed_image_types.find(attachment_content_type);
    const bool is_attachment_allowed = allowed_img_type_it != allowed_image_types.end();

    if (!is_attachment_allowed){
        dpp::message msg{ "Error: type " + attachment_content_type + " not supported" };
        if (!broadcast) {
            msg.set_flags(dpp::m_ephemeral);
        }
        event.reply(msg);
        co_return;
    }

    // Send a "<bot> is thinking..." message, to wait on later so we can edit
    dpp::async thinking = event.co_thinking(!broadcast);
    // Download and co_await the result
    dpp::http_request_completion_t response = co_await cluster->co_request(attachment_url, dpp::m_get);
    if (response.status != 200){
        co_await thinking;
        event.edit_response("Error: could not download the attachment");
    }
    else{
        dpp::emoji emoji(emoji_name);
        emoji.load_image(response.body, allowed_img_type_it->second);

        dpp::confirmation_callback_t confirmation = co_await cluster->co_guild_emoji_create(event.command.guild_id, emoji);
        co_await thinking;
        if (confirmation.is_error()){
            event.edit_response("Error: could not add emoji: " + confirmation.get_error().message);
        }else {
            event.edit_response("Successfully added " + confirmation.get<dpp::emoji>().get_mention());
        }
    }
    co_return;
}

dpp::slashcommand mln::add_emoji::get_command(dpp::cluster &bot){
    return dpp::slashcommand(mln::add_emoji::get_command_name(), "Add an emoji", bot.me.id)
            .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
            .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true))
            .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel"));
}

std::string mln::add_emoji::get_command_name(){
    return "add_emoji";
}
