#include "commands/slash/add_emoji.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/constants.h"

mln::add_emoji::add_emoji(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("add_emoji", "Add an emoji", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)
        .add_option(dpp::command_option(dpp::co_attachment, "file", "Select an image", true))
        .add_option(dpp::command_option(dpp::co_string, "name", "Name of the emoji to add", true)
            .set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_emoji())))
            .set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_emoji()))))
        .add_option(dpp::command_option(dpp::co_boolean, "broadcast", "Broadcast result to the channel", false)))){}

dpp::task<void> mln::add_emoji::command(const dpp::slashcommand_t& event_data){
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(!broadcast);

    //Retrieve guild data
    std::tuple<dpp::guild*, dpp::guild> guild_pair = co_await mln::utility::get_guild(event_data, delta()->bot);
    dpp::guild* cmd_guild = std::get<0>(guild_pair);
    if (cmd_guild == nullptr) {
        //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
        cmd_guild = &std::get<1>(guild_pair);
    }

    //Retrieve channel data
    std::tuple<dpp::channel*, dpp::channel> channel_pair = co_await mln::utility::get_channel(event_data, event_data.command.channel_id, delta()->bot);
    dpp::channel* cmd_channel = std::get<0>(channel_pair);
    if (cmd_channel == nullptr) {
        //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
        cmd_channel = &std::get<1>(channel_pair);
    }

    //If we failed to find the guild the command originated from or the channel, we return an error
    if (cmd_channel->id == 0 || cmd_guild->id == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve guild and channel data!");
        co_return;
    }

    const std::optional<dpp::guild_member> bot = co_await mln::utility::get_member(event_data, cmd_guild, event_data.command.application_id, delta()->bot);
    const std::optional<dpp::guild_member> usr = co_await mln::utility::get_member(event_data, cmd_guild, event_data.command.usr.id, delta()->bot);

    //If we failed to find the bot or the usr, we return an error
    if (!bot.has_value() || bot->user_id == 0 || !usr.has_value() || usr->user_id == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve bot and user data!");
        co_return;
    }

    const bool are_perms_valid = mln::utility::check_permissions(cmd_guild, cmd_channel, bot, dpp::permissions::p_manage_emojis_and_stickers) &&
        mln::utility::check_permissions(cmd_guild, cmd_channel, {bot, usr}, dpp::permissions::p_use_application_commands);
    if (!are_perms_valid) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed to add new emoji, either the bot or the user do not possess the required permissions!");
        co_return;
    }

    const dpp::confirmation_callback_t emojis = co_await delta()->bot.co_guild_emojis_get(cmd_guild->id);
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

        const dpp::confirmation_callback_t confirmation = co_await delta()->bot.co_guild_emoji_create(cmd_guild->id, emoji);
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            confirmation.is_error() ? "Error: could not add emoji: " + confirmation.get_error().human_readable : "Successfully added " + confirmation.get<dpp::emoji>().get_mention(),
            {false, dpp::loglevel::ll_debug});
    }
}