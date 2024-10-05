#include "commands/slash/add_emoji.h"
#include "commands/slash/base_slashcommand.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/http_err.h"
#include "utility/json_err.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/emoji.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>
#include <dpp/queues.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

mln::add_emoji::add_emoji(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("add_emoji"), "Add an emoji", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
        .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_emoji())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_emoji()))))
    ) } {}

dpp::job mln::add_emoji::command(dpp::slashcommand_t event_data) const {
    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed add_emoji, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_think(lite_data, true, false, {});

    if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
        co_await mln::response::co_respond(lite_data, "Failed add_emoji, discord error!", true, "Failed add_emoji, the event does not hold the correct type of data for parameters!");
        co_return;
    }

    const dpp::confirmation_callback_t emojis = co_await bot().co_guild_emojis_get(lite_data.guild_id);
    if (emojis.is_error()) {
        const dpp::error_info err = emojis.get_error();
        static const std::string s_err_text = "Failed to add new emoji, error while attempting to get server emoji list!";

        co_await mln::response::co_respond(lite_data, s_err_text, true, std::format("{} Error: [{}], details: [{}].", s_err_text, mln::get_json_err_text(err.code), err.human_readable));

        co_return;
    }

    if (!std::holds_alternative<dpp::emoji_map>(emojis.value)) {
        co_await mln::response::co_respond(lite_data, "Failed to retrieve server emoji list!", true, "Failed to retrieve server emoji list!");
        co_return;
    }

    const dpp::command_value& emoji_param = event_data.get_parameter("name");
    const std::optional<std::string> emoji_name = co_await mln::utility::check_text_validity(emoji_param, lite_data, false, 
        mln::constants::get_min_characters_emoji(), mln::constants::get_max_characters_emoji(), "emoji name");

    if (!emoji_name.has_value()) {
        co_return;
    }

    const dpp::emoji_map& emojis_map = std::get<dpp::emoji_map>(emojis.value);
    for (const std::pair<dpp::snowflake, dpp::emoji>& pair : emojis_map) {
        if (pair.second.name == emoji_name.value()) {
            co_await mln::response::co_respond(lite_data, "Failed to add new emoji, an emoji with the same given name already exist on this server!", false, {});

            co_return;
        }
    }

    const dpp::command_value& file_param = event_data.get_parameter("file");
    const dpp::snowflake file_id = std::holds_alternative<dpp::snowflake>(file_param) ? std::get<dpp::snowflake>(file_param) : dpp::snowflake{ 0 };

    if (file_id == 0) {
        co_await mln::response::co_respond(lite_data, "Failed to retrieve file id parameter!", true, "Failed to retrieve file id parameter!");
        co_return;
    }

    const auto it = event_data.command.resolved.attachments.find(file_id);
    if (it == event_data.command.resolved.attachments.end()) {
        static const std::string s_err_text = "Failed to add new emoji, the bot couldn't retrieve the uploaded attachment!";
        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

        co_return;
    }

    const dpp::attachment attachment = it->second;
    const std::string attachment_content_type = attachment.content_type;
    const std::string attachment_url = attachment.url;

    if (attachment_url.empty()) {
        static const std::string s_err_text = "Failed to add new emoji, couldn't retrieve the attachment url!";
        co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

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
        co_await mln::response::co_respond(lite_data, std::format("Failed to add new emoji, couldn't retrieve the attachment url! Type [{}] is not supported for emojis!", attachment_content_type), false, {});

        co_return;
    }

    // Download and co_await the result
    const dpp::http_request_completion_t response = co_await bot().co_request(attachment_url, dpp::m_get);
    if (response.status != 200 || response.body.empty()){

        static const std::string s_err_text = "Error: could not download the attachment!";
        co_await mln::response::co_respond(lite_data, s_err_text, true,
            std::format("Error: could not download the attachment! Status: [{}], error: [{}].", mln::get_http_err_text(response.status), mln::get_dpp_http_err_text(response.error)));

        co_return;
    }

    dpp::emoji emoji(emoji_name.value());
    emoji.load_image(response.body, allowed_img_type_it->second);

    const dpp::confirmation_callback_t confirmation = co_await bot().co_guild_emoji_create(lite_data.guild_id, emoji);

    if (confirmation.is_error()) {
        const dpp::error_info err = confirmation.get_error();
        static const std::string s_err_text = "Error: could not add emoji!";
        co_await mln::response::co_respond(lite_data, s_err_text, true,
            std::format("{} Error: [{}], details: [{}].", s_err_text, mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    co_await mln::response::co_respond(lite_data, "Emoji added!", false, "Failed add_emoji command conclusion reply!");
}

std::optional<std::function<void()>> mln::add_emoji::job(dpp::slashcommand_t) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::add_emoji::use_job() const
{
    return false;
}
