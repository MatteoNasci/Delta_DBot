#include "commands/add_emoji.h"

#include <dpp/snowflake.h>
#include <dpp/message.h>
#include <dpp/coro.h>

#include <variant>

dpp::task<void> add_emoji::command(bot_delta_data_t &data, const dpp::slashcommand_t &event){
    dpp::cluster *cluster = event.from->creator;
    // Retrieve parameter values
    dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
    std::string emoji_name = std::get<std::string>(event.get_parameter("name"));
    // Get the attachment from the resolved list
    const dpp::attachment &attachment = event.command.get_resolved_attachment(file_id);

    // For simplicity for this example we only support PNG
    if (attachment.content_type != "image/png") {
        // While event.co_reply is available, we can just use event.reply, as we will exit the command anyway and don't need to wait on the result
        event.reply("Error: type " + attachment.content_type + " not supported");
        co_return;
    }

    // Send a "<bot> is thinking..." message, to wait on later so we can edit
    dpp::async thinking = event.co_thinking(false);
    // Download and co_await the result
    dpp::http_request_completion_t response = co_await cluster->co_request(attachment.url, dpp::m_get);
    if (response.status != 200) { // Page didn't send the image
        co_await thinking; // Wait for the thinking response to arrive so we can edit
        event.edit_response("Error: could not download the attachment");
    }
    else {
        // Load the image data in a dpp::emoji
        dpp::emoji emoji(emoji_name);
        emoji.load_image(response.body, dpp::image_type::i_png);
        // Create the emoji and co_await the response
        dpp::confirmation_callback_t confirmation = co_await cluster->co_guild_emoji_create(event.command.guild_id, emoji);
        co_await thinking; // Wait for the thinking response to arrive so we can edit
        if (confirmation.is_error())
            event.edit_response("Error: could not add emoji: " + confirmation.get_error().message);
        else // Success
            event.edit_response("Successfully added " + confirmation.get<dpp::emoji>().get_mention()); // Show the new emoji
    }
    co_return;
}

dpp::slashcommand add_emoji::get_command(dpp::cluster &bot)
{
    return dpp::slashcommand(add_emoji::get_command_name(), "Add an emoji", bot.me.id)
            .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
            .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true));
}

std::string add_emoji::get_command_name()
{
    return "add_emoji";
}
