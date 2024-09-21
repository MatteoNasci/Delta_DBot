#include "commands/slash/add_emoji.h"
#include "utility/utility.h"
#include "utility/constants.h"
#include "utility/caches.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/json_err.h"
#include "utility/http_err.h"
#include "utility/reply_log_data.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

#include <format>

mln::add_emoji::add_emoji(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand("add_emoji", "Add an emoji", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
        .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_emoji())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_emoji()))))
    ) } {}

dpp::task<void> mln::add_emoji::command(const dpp::slashcommand_t& event_data) const {
    if (mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed thinking for add_emoji!");
        co_return;
    }

    const reply_log_data_t reply_log_data{ &event_data, &bot(), false };
    //Retrieve guild data
    const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_full(event_data.command.guild_id, reply_log_data);
    if (!guild.has_value()) {
        co_return;
    }

    const dpp::confirmation_callback_t emojis = co_await bot().co_guild_emojis_get(guild.value()->id);
    if (emojis.is_error()) {
        const dpp::error_info err = emojis.get_error();

        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Failed to add new emoji, error while attempting to get server emoji list!"), bot(), &event_data, 
            std::format("Failed to add new emoji, error while attempting to get server emoji list! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    const std::string emoji_name = std::get<std::string>(event_data.get_parameter("name"));

    const dpp::emoji_map& emojis_map = emojis.get<dpp::emoji_map>();
    for (const std::pair<dpp::snowflake, dpp::emoji>& pair : emojis_map) {
        if (pair.second.name == emoji_name) {
            mln::utility::conf_callback_is_error(
                co_await mln::response::make_response(false, event_data, "Failed to add new emoji, an emoji with the same given name already exist on this server!"), bot());
            co_return;
        }
    }

    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const auto it = event_data.command.resolved.attachments.find(file_id);
    if (it == event_data.command.resolved.attachments.end()) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Failed to add new emoji, the bot couldn't retrieve the uploaded attachment!"), bot(), &event_data, 
            "Failed to add new emoji, the bot couldn't retrieve the uploaded attachment!");
        co_return;
    }

    const dpp::attachment attachment = it->second;
    const std::string attachment_content_type = attachment.content_type;
    const std::string attachment_url = attachment.url;

    if (attachment_url.empty()) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Failed to add new emoji, couldn't retrieve the attachment url!"), bot(), &event_data,
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
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, std::format("Failed to add new emoji, couldn't retrieve the attachment url! Type [{}] is not supported for emojis!", attachment_content_type)), bot());
        co_return;
    }

    // Download and co_await the result
    const dpp::http_request_completion_t response = co_await bot().co_request(attachment_url, dpp::m_get);
    if (response.status != 200){
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Error: could not download the attachment!"), bot(), &event_data,
            std::format("Error: could not download the attachment! Status: [{}], error: [{}].", mln::get_http_err_text(response.status), mln::get_dpp_http_err_text(response.error)));
        co_return;
    }

    dpp::emoji emoji(emoji_name);
    emoji.load_image(response.body, allowed_img_type_it->second);

    const dpp::confirmation_callback_t confirmation = co_await bot().co_guild_emoji_create(guild.value()->id, emoji);

    if (confirmation.is_error()) {
        const dpp::error_info err = confirmation.get_error();

        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Error: could not add emoji!"), bot(), &event_data,
            std::format("Error: could not add emoji! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Emoji added!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed add_emoji command conclusion reply!");
    }
}