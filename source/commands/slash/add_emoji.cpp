#include "commands/slash/add_emoji.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/constants.h"
#include "utility/caches.h"
#include "utility/perms.h"

mln::add_emoji::add_emoji(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("add_emoji", "Add an emoji", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
        .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_emoji())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_emoji()))))
    )){}

dpp::task<void> mln::add_emoji::command(const dpp::slashcommand_t& event_data){
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    //Retrieve guild data
    std::optional<std::shared_ptr<const dpp::guild>> guild = mln::caches::get_guild(event_data.command.guild_id);
    if (!guild.has_value()) {
        guild = co_await mln::caches::get_guild_task(event_data.command.guild_id);
        if (!guild.has_value()) {
            //Error can't find guild
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to retrieve guild data! guild_id: "
                + std::to_string(event_data.command.guild_id));
            co_return;
        }
    }

    const dpp::confirmation_callback_t emojis = co_await delta()->bot.co_guild_emojis_get(guild.value()->id);
    if (emojis.is_error()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed to add new emoji, error while attempting to get server emoji list! Error: " + emojis.get_error().human_readable);
        co_return;
    }

    const std::string emoji_name = std::get<std::string>(event_data.get_parameter("name"));

    const dpp::emoji_map& emojis_map = emojis.get<dpp::emoji_map>();
    for (const std::pair<dpp::snowflake, dpp::emoji>& pair : emojis_map) {
        if (pair.second.name == emoji_name) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to add new emoji, an emoji with the same given name already exist on this server!", {false, dpp::loglevel::ll_debug});
            co_return;
        }
    }
    //TODO remove all get_resolved, use command.resolved

    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const auto it = event_data.command.resolved.attachments.find(file_id);
    if (it == event_data.command.resolved.attachments.end()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed to add new emoji, the bot couldn't retrieve the uploaded attachment!");
        co_return;
    }

    const dpp::attachment attachment = it->second;
    const std::string attachment_content_type = attachment.content_type;
    const std::string attachment_url = attachment.url;

    if (attachment_url.empty()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed to add new emoji, couldn't retrieve the attachment url!");
        co_return;
    }

    static const std::unordered_map<std::string, dpp::image_type> s_allowed_image_types{
        {"image/png", dpp::image_type::i_png},
        {"image/gif", dpp::image_type::i_gif},
        {"image/jpeg", dpp::image_type::i_jpg},
        {"image/webp", dpp::image_type::i_webp}
    };

    const auto& allowed_img_type_it = s_allowed_image_types.find(attachment_content_type);
    const bool is_attachment_allowed = allowed_img_type_it != s_allowed_image_types.end();

    if (!is_attachment_allowed){
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Error: type " + attachment_content_type + " not supported for emojis!");
        co_return;
    }

    // Download and co_await the result
    const dpp::http_request_completion_t response = co_await delta()->bot.co_request(attachment_url, dpp::m_get);
    if (response.status != 200){
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Error: could not download the attachment! Error: " + std::to_string(response.error));
    }
    else{
        dpp::emoji emoji(emoji_name);
        emoji.load_image(response.body, allowed_img_type_it->second);

        const dpp::confirmation_callback_t confirmation = co_await delta()->bot.co_guild_emoji_create(guild.value()->id, emoji);
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            confirmation.is_error() ? "Error: could not add emoji: " + confirmation.get_error().human_readable : "Successfully added " + confirmation.get<dpp::emoji>().get_mention(),
            {false, dpp::loglevel::ll_debug});
    }
}