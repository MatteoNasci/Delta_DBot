#include "commands/slash/add_emoji.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::add_emoji::add_emoji(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("add_emoji", "Add an emoji", delta->bot.me.id)
        .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
        .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_emoji())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_emoji()))))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel", false)))){}

dpp::task<void> mln::add_emoji::command(const dpp::slashcommand_t& event_data){
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    const dpp::permission bot_perms = event_data.command.app_permissions;
    const dpp::permission& usr_perms = event_data.command.get_resolved_permission(event_data.command.usr.id);
    const bool are_perms_valid = bot_perms.can(dpp::permissions::p_manage_emojis_and_stickers) && 
                                usr_perms.can(dpp::permissions::p_manage_emojis_and_stickers);

    if (!are_perms_valid) {
        co_await thinking;
        event_data.edit_response("Failed to add new emoji, either the bot or the user do not possess the required permissions!");
        co_return;
    }

    if (event_data.from == nullptr) {
        co_await thinking;
        event_data.edit_response("Failed to add new emoji, Impossible to retrieve the user's cluster!");
        co_return;
    }

    dpp::cluster *const cluster = event_data.from->creator;

    if (cluster == nullptr) {
        co_await thinking;
        event_data.edit_response("Failed to add new emoji, Impossible to retrieve the user's cluster!");
        co_return;
    }
    
    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const std::string emoji_name = std::get<std::string>(event_data.get_parameter("name"));

    const dpp::confirmation_callback_t emojis = co_await cluster->co_guild_emojis_get(event_data.command.guild_id);
    if (emojis.is_error()) {
        co_await thinking;
        event_data.edit_response("Failed to add new emoji, error while attempting to get server emoji list! Error: " + emojis.get_error().human_readable);
        co_return;
    }

    const dpp::emoji_map& emojis_map = emojis.get<dpp::emoji_map>();
    for (const std::pair<dpp::snowflake, dpp::emoji>& pair : emojis_map) {
        if (pair.second.name == emoji_name) {
            co_await thinking;
            event_data.edit_response("Failed to add new emoji, an emoji with the same given name already exist on this server!");
            co_return;
        }
    }
    
    const dpp::attachment &attachment = event_data.command.get_resolved_attachment(file_id);
    const std::string attachment_content_type = attachment.content_type;
    const std::string attachment_url = attachment.url;

    if (attachment_url.empty()) {
        co_await thinking;
        event_data.edit_response("Failed to add new emoji, couldn't retrieve the attachment url!");
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
        co_await thinking;
        event_data.edit_response("Error: type " + attachment_content_type + " not supported for emojis!");
        co_return;
    }

    // Download and co_await the result
    const dpp::http_request_completion_t response = co_await cluster->co_request(attachment_url, dpp::m_get);//event_data lifetime is guaranteed by the event_handler on the dpp side
    if (response.status != 200){
        co_await thinking;
        event_data.edit_response("Error: could not download the attachment! Error: " + std::to_string(response.error));
    }
    else{
        dpp::emoji emoji(emoji_name);
        emoji.load_image(response.body, allowed_img_type_it->second);

        const dpp::confirmation_callback_t confirmation = co_await cluster->co_guild_emoji_create(event_data.command.guild_id, emoji);
        co_await thinking;
        if (confirmation.is_error()){
            event_data.edit_response("Error: could not add emoji: " + confirmation.get_error().human_readable);
        }else {
            event_data.edit_response("Successfully added " + confirmation.get<dpp::emoji>().get_mention());
        }
    }
}