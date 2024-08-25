#include "commands/slash/db/db_insert.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/constants.h"

#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/permissions.h>

mln::db_insert::db_insert(bot_delta* const delta) : base_db_command(delta), data{.valid_stmt = true}{

    const mln::db_result res1 = delta->db.save_statement(
        "INSERT OR ABORT INTO storage(guild_id, name, url, desc, user_id) VALUES(:GGG, :NNN, :LLL, :DDD, :UUU) RETURNING user_id;", data.saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert url stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":LLL", data.saved_param_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        mln::db_result res15 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert url stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_insert::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    
    static const std::unordered_map<mln::db_command_type, std::function<dpp::task<void>(mln::db_insert*, const dpp::slashcommand_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>> s_allowed_subcommands{
        {mln::db_command_type::url, &mln::db_insert::command_url},
        {mln::db_command_type::text, &mln::db_insert::command_text},
        {mln::db_command_type::file, &mln::db_insert::command_file},
        {mln::db_command_type::help, &mln::db_insert::command_help},
    };

    //Find the command variant and execute it. If no valid command variant found return an error
    const auto it_func = s_allowed_subcommands.find(type);
    if (it_func == s_allowed_subcommands.end()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, the given sub_command is not supported!");
        co_return;
    }

    //If the query statement was not saved correctly, return an error
    if (!data.valid_stmt) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    co_await it_func->second(this, event_data, cmd_data, thinking);
}

mln::db_init_type_flag mln::db_insert::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::file, db_init_type_flag::all},
        {db_command_type::text, db_init_type_flag::cmd_data | db_init_type_flag::dump_channel},
        {db_command_type::url, db_init_type_flag::all},
        {db_command_type::help, db_init_type_flag::cmd_data},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}

dpp::task<void> mln::db_insert::command_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Verify if given url is a valid discord message url
    const std::string input_url = std::get<std::string>(event_data.get_parameter("url"));
    uint64_t url_guild_id{0}, url_channel_id{0}, url_message_id{0};
    const bool is_url_msg = mln::utility::extract_message_url_data(input_url, url_guild_id, url_channel_id, url_message_id);

    if (!is_url_msg) {
        //Verify if the given url is a valid discord attachment url
        const bool is_url_attach = mln::utility::extract_generic_attachment_url_data(input_url, url_guild_id, url_channel_id);

        //The given url is neither a message url nor an attachment url, return an error
        if (!is_url_attach) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed url parsing, the given url in neither pointing to a valid message nor to a valid attachment!", {true, dpp::loglevel::ll_debug});
            co_return;
        }

        //Handle attachment url
        co_await manage_attach_url(event_data, cmd_data, input_url, thinking);
    } else {
        msg_url_t url_data{.guild_id = url_guild_id, .channel_id = url_channel_id, .message_id = url_message_id};
        //Handle msg url
        co_await manage_msg_url(event_data, cmd_data, url_data, thinking);
    }
}
dpp::task<void> mln::db_insert::command_text(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const uint32_t s_max_characters_allowed = std::min(mln::constants::get_max_characters_modal_component(), std::max(mln::constants::get_max_characters_embed_description(), std::max(mln::constants::get_max_characters_embed_field_value(), mln::constants::get_max_characters_reply_msg())));
    static const uint32_t s_max_characters_allowed_field = std::min(static_cast<uint32_t>(mln::constants::get_max_characters_embed_field_value()), s_max_characters_allowed);
    static const uint32_t s_max_characters_allowed_msg = std::min(static_cast<uint32_t>(mln::constants::get_max_characters_reply_msg()), s_max_characters_allowed);
    static const uint32_t s_max_characters_allowed_desc = std::min(static_cast<uint32_t>(mln::constants::get_max_characters_embed_description()), s_max_characters_allowed);
    static const uint32_t s_max_characters_allowed_final_field = std::min(static_cast<uint32_t>(mln::constants::get_max_characters_embed_total()) - s_max_characters_allowed_desc - s_max_characters_allowed_field, s_max_characters_allowed);
    //Modals have a max of 5 minutes of uptime without being submitted
    static const uint64_t s_max_time = 60 * 5;
    //Create the standard modal. Modal has a max of 5 rows and 4k characters per row
    static const dpp::interaction_modal_response s_modal = dpp::interaction_modal_response{}
        .set_title("Insert the text to store in the database.")

        .add_component(
        dpp::component{}
        .set_label("Message content")
        .set_id("0")
        .set_min_length(0)
        .set_max_length(s_max_characters_allowed_msg)
        .set_text_style(dpp::text_style_type::text_paragraph)
        .set_type(dpp::component_type::cot_text)
        .set_placeholder("Text that will be shown as part of a discord message content"))
        
        .add_row()
        
        .add_component(
        dpp::component{}
        .set_label("Embed description content")
        .set_id("1")
        .set_min_length(0)
        .set_max_length(s_max_characters_allowed_desc)
        .set_text_style(dpp::text_style_type::text_paragraph)
        .set_type(dpp::component_type::cot_text)
        .set_placeholder("Text that will be shown as part of a discord message embed description"))
        
        .add_row()

        .add_component(
        dpp::component{}
        .set_label("Embed field 1 content")
        .set_id("2")
        .set_min_length(0)
        .set_max_length(s_max_characters_allowed_field)
        .set_text_style(dpp::text_style_type::text_paragraph)
        .set_type(dpp::component_type::cot_text)
        .set_placeholder("Text that will be shown as part of a discord message embed field value"))
        
        .add_row()
        
        .add_component(
        dpp::component{}
        .set_label("Embed field 2 content")
        .set_id("3")
        .set_min_length(0)
        .set_max_length(s_max_characters_allowed_final_field)
        .set_text_style(dpp::text_style_type::text_paragraph)
        .set_type(dpp::component_type::cot_text)
        .set_placeholder("Text that will be shown as part of a discord message embed field value"));

    //We get a copy of the 'standard' modal, and give it an unique id
    dpp::interaction_modal_response modal{s_modal};
    const std::string modal_id = "modal_" + std::to_string(event_data.command.id);
    modal.set_custom_id(modal_id);

    //Try to reply with a dialog box, return an error if failure
    dpp::confirmation_callback_t dialog_conf = co_await event_data.co_dialog(modal);
    if (dialog_conf.is_error()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to create dialog! Error: " + dialog_conf.get_error().human_readable);
        co_return;
    }
    
    //Wait for a max amount of time for the dialog submission
    const auto &result = co_await dpp::when_any{
        delta()->bot.co_sleep(s_max_time),
        delta()->bot.on_form_submit.when([&modal_id](const dpp::form_submit_t& event_data) {return event_data.custom_id == modal_id;})};

    //If the timer run out return an error
    if (result.index() == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to insert text data in time!");
        co_return;
    }

    //If an exception occurred return an error
    if (result.is_exception()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to insert text data, unknown error occurred!");
        co_return;
    }

    //It was suggested to copy the event from documentation of ::when
    dpp::form_submit_t form_data = result.get<1>();
    if (form_data.cancelled) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to insert text data! Internal error!");
        co_return;
    }

    thinking = form_data.co_thinking(true);
    //From here on we need a normal reply instead of an edit_response sicne we are doing a dialog form
    //Get parameters, make sure there is at least 1 character. Remember the parameters are optional!
    dpp::message msg = dpp::message{}.set_guild_id(cmd_data.cmd_guild->id).set_channel_id(cmd_data.dump_channel->id);
    dpp::embed embed{};
    for (const dpp::component& component : form_data.components) {
        if (component.components.size() == 0) {
            continue;
        }
        const dpp::component field = component.components[0];

        if (!std::holds_alternative<std::string>(field.value)) {
            continue;
        }

        if (field.custom_id == "0") {
            msg.set_content(std::get<std::string>(field.value));
        } else if (field.custom_id == "1" && std::get<std::string>(field.value).size() > 0) {
            embed.set_description(std::get<std::string>(field.value));
        } else if (field.custom_id == "2" && std::get<std::string>(field.value).size() > 0) {
            embed.add_field("", std::get<std::string>(field.value));
        } else if (field.custom_id == "3" && std::get<std::string>(field.value).size() > 0) {
            embed.add_field("", std::get<std::string>(field.value));
        }
    }

    //Add the embed to the msg if something has been written into it
    if (embed.description.size() > 0 || embed.fields.size() > 0) {
        msg.add_embed(embed);
    } else {
        //If embeds are empty AND the msg content is empty return an error
        if (msg.content.size() == 0) {
            co_await mln::utility::co_conclude_thinking_response(thinking, form_data, delta()->bot, "Failed command, there needs to be at least 1 character in total in the overall provided text!");
            co_return;
        }
    }

    //If the bot cannot send messages in the dump channel return an error
    const bool create_msg_permission = mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_send_messages);
    if (!create_msg_permission) {
        co_await mln::utility::co_conclude_thinking_response(thinking, form_data, delta()->bot, "Failed command, the bot doesn't have permission to write into the dump channel!");
        co_return;
    }

    //Return an error if we fail to send the message
    const dpp::confirmation_callback_t create_msg_conf = co_await delta()->bot.co_message_create(msg);
    if (create_msg_conf.is_error()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, form_data, delta()->bot, "Failed command, the bot couldn't create the message in the dump channel! Error: " + create_msg_conf.get_error().human_readable);
        co_return;
    }

    dpp::message created = create_msg_conf.get<dpp::message>();
    //Execute the database query using the created message url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, form_data, cmd_data, created.get_url(), thinking);//TODO make sure thinking is still valid here, we might have co_awaited it
    //Errors are handled by execute_query
    if (!is_query_success) {
        co_return;
    }

    //Update caches
    delta()->messages_cache.add_element(created.id, std::move(created));
    delta()->show_all_cache.remove_element(cmd_data.cmd_guild->id);
    delta()->show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    co_await mln::utility::co_conclude_thinking_response(thinking, form_data, delta()->bot, "Database operation successful!", {false, dpp::loglevel::ll_debug});
}
dpp::task<void> mln::db_insert::command_file(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Find given file from resolved elements, if not found return an error
    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const auto it = event_data.command.resolved.attachments.find(file_id);
    if (it == event_data.command.resolved.attachments.end()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed database operation, failed to retrieve given file!");
        co_return;
    }

    //Verify bot permission
    if (!mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, {dpp::permissions::p_attach_files, dpp::permissions::p_send_messages})) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, the bot doesn't have the permission to upload attachments or send messages in the dump channel!");
        co_return;
    }

    //Download attachment, return error on failure
    const dpp::http_request_completion_t download_res = co_await delta()->bot.co_request(it->second.url, dpp::http_method::m_get, "", it->second.content_type);
    if (download_res.status != 200) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
            "Failed database operation, failed to download given file! Error: " + std::to_string(download_res.error) + ", status: " + std::to_string(download_res.status));
        co_return;
    }

    //Create and send message into dump channel, return error on fail
    dpp::message message_to_store = dpp::message{}
        .add_file(it->second.filename, download_res.body, it->second.content_type)
        .set_channel_id(cmd_data.dump_channel->id)
        .set_guild_id(cmd_data.cmd_guild->id);

    const dpp::confirmation_callback_t create_res = co_await delta()->bot.co_message_create(message_to_store);
    if (create_res.is_error()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed database operation, failed to send file to dump channel! Error: " + create_res.get_error().human_readable);
        co_return;
    }

    dpp::message created = create_res.get<dpp::message>();
    if (!co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, created.get_url(), thinking)) {
        co_return;
    }

    //Update caches
    delta()->messages_cache.add_element(created.id, std::move(created));
    delta()->show_all_cache.remove_element(cmd_data.cmd_guild->id);
    delta()->show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Database operation successful!", {false, dpp::loglevel::ll_debug});
}
dpp::task<void> mln::db_insert::command_help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db insert` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db insert` set of commands is used to insert new records into the database. The new record will be identified by the name supplied by the user, and it will be retrievable through the command `/db select single chosen_name`.

Each record can be supplied with a description (max 100 characters), which will allow the user and other server members to quickly understand what is stored in the record while using the retrieval commands `/db select` and `/db show`.

Attempting to insert a new record using a name that is already being used by another record will result in an error.

If a dump channel has been set for the current server, all the stored records will be kept in the dump channel for storage purposes.

**Types of insert:**

- **/db insert text**  
  *Parameters:* name[text, required], description[text, optional].  
  This command will ask you to provide a name for the new record and, optionally, a description for it.  
  Once that is done, a dialog menu will open, allowing the user to insert up to 8000 characters in text form to be stored in the database. The dialog menu contains 4 fields with varying character limits that can be filled.  
  The user has up to 5 minutes to insert the data and confirm it. If time runs out or if the menu is dismissed, the command will terminate.  
  Once the text is submitted (and no error occurs), it will be stored in the dump channel and will be available for retrieval.

- **/db insert file**  
  *Parameters:* name[text, required], file[attachment, required], description[text, optional].  
  This command will ask you to provide a name for the new record, an attachment, and, optionally, a description for it.  
  Once that is done, the bot will attempt to create a new record and save the file in the dump channel. Once all is done (and no error occurs), the new record containing the given file will be stored and available for retrieval.

- **/db insert url**  
  *Parameters:* name[text, required], url[text, required], description[text, optional].  
  This command will ask you to provide a name for the new record, a URL link to a Discord message/attachment, and, optionally, a description for it.  
  The accepted URLs have the following forms:  
  - `https://discord.com/channels/[number]/[number]/[number]` (for links to Discord messages).  
  - `https://cdn.discordapp.com/attachments/[number]/[number]/...` (for links to Discord attachments).  
  - `https://cdn.discordapp.com/ephemeral-attachments/[number]/[number]/...` (for links to Discord ephemeral attachments).  
  If the supplied URL is valid and no error occurs, the message/attachment will be saved in the dump channel and will be ready for retrieval.  
  This is the most versatile and powerful command in the `insert` category since it allows the user to store anything (even from other servers) as long as the bot has access to it. Using this command allows the user to store messages with multiple attachments and text content (something that cannot be done by the other two `insert` variants).)"""));

    event_data.reply(s_info);
    co_return;
}


dpp::task<void> mln::db_insert::manage_attach_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const std::string& url, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    const bool create_msg_permission = mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_send_messages);
    //Return an error if the bot is not allowed to send messages in the dump channel
    if (!create_msg_permission) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, the bot doesn't have the permission to send messages in the dump channel!", {true, dpp::loglevel::ll_debug});
        co_return;
    }

    //Make an API request to create a message in the dump channel with the given attachment url
    dpp::message msg{url};
    const dpp::confirmation_callback_t msg_create_result = co_await delta()->bot.co_message_create(msg.set_channel_id(cmd_data.dump_channel->id).set_guild_id(cmd_data.cmd_guild->id));

    //If message creation failed return an error
    if (msg_create_result.is_error()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
            "Failed to create message in the designated dump channel! Error: " + msg_create_result.get_error().human_readable + ".");
        co_return;
    }

    dpp::message created = msg_create_result.get<dpp::message>();
    //Execute the database query using the created message url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, created.get_url(), thinking);
    //Errors are handled by execute_query
    if (!is_query_success) {
        co_return;
    }

    //Update caches
    delta()->messages_cache.add_element(created.id, std::move(created));
    delta()->show_all_cache.remove_element(cmd_data.cmd_guild->id);
    delta()->show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    //Return a success reply to the user
    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Database operation successful!", {false, dpp::loglevel::ll_debug});
}
dpp::task<void> mln::db_insert::manage_msg_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const mln::msg_url_t& url_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking){
    //Retrieve url guild data
    dpp::guild* url_guild = cmd_data.cmd_guild;
    std::tuple<dpp::guild*, dpp::guild> url_guild_pair;
    if (url_data.guild_id != cmd_data.cmd_guild->id) {
        url_guild_pair = co_await mln::utility::get_guild(url_data.guild_id, delta()->bot);
        url_guild = std::get<0>(url_guild_pair);
        if (url_guild == nullptr) {
            //Make sure this pointer stays alive to the end of the function at least. Make sure to co_await the manage_... functions at the end
            url_guild = &std::get<1>(url_guild_pair);
        }
    }

    //Retrieve url channel data
    dpp::channel* url_channel = cmd_data.cmd_channel;
    std::tuple<dpp::channel*, dpp::channel> url_channel_pair;
    if (url_data.channel_id != cmd_data.cmd_channel->id) {
        url_channel_pair = co_await mln::utility::get_channel(event_data, url_data.channel_id, delta()->bot);
        url_channel = std::get<0>(url_channel_pair);
        if (url_channel == nullptr) {
            //Make sure this pointer stays alive to the end of the function at least. Make sure to co_await the manage_... functions at the end
            url_channel = &std::get<1>(url_channel_pair);
        }
    }

    //If we failed to find the url guild or the url channel, we return an error
    if (url_channel->id == 0 || url_guild->id == 0) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve url guild and url channel data!");
        co_return;
    }
    
    //Retrieve message from cache (if present) using the given url message id value
    const std::optional<dpp::message> opt_cached_msg = delta()->messages_cache.get_element(url_data.message_id);
    const bool cached_msg_found = opt_cached_msg.has_value();

    //Prepare output values for following if/else
    dpp::message created_msg{};
    std::string final_url{};

    //If we found a valid entry from the cache we use that message
    if (cached_msg_found) {

        //We create a new storage msg if the cache entry message is located in a different guild or in a channel different than the local dump channel. Note that if a dump channel is not set, we use the local channel as the dump channel.
        const bool need_to_create_storage = opt_cached_msg->channel_id != cmd_data.dump_channel->id || opt_cached_msg->guild_id != cmd_data.cmd_guild->id;
        if (need_to_create_storage) {
            const bool create_msg_permission = opt_cached_msg->tts ?
                mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, {dpp::permissions::p_send_messages, dpp::permissions::p_send_tts_messages}) :
                mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_send_messages);
            //Return an error if the bot is not allowed to send messages in the dump channel
            if (!create_msg_permission) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed command, the bot doesn't have the permission to send messages in the dump channel!", {true, dpp::loglevel::ll_debug});
                co_return;
            }

            //Create a copy of the cached message in the dump channel
            const dpp::confirmation_callback_t msg_create_result = 
                co_await delta()->bot.co_message_create(dpp::message{opt_cached_msg.value()}.set_channel_id(cmd_data.dump_channel->id).set_guild_id(cmd_data.cmd_guild->id));

            //Return an error if we couldn't create the copy
            if (msg_create_result.is_error()) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
                    "Failed to create message in the designated dump channel! Error: " + msg_create_result.get_error().human_readable + ".");
                co_return;
            }

            created_msg = std::move(msg_create_result.get<dpp::message>());
            final_url = created_msg.get_url();
        } else {
            //The cached message is already located in the dump channel, we use its url directly
            final_url = opt_cached_msg->get_url();
        }

    } else { 
        const bool retrieve_create_msg_permission = 
            mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_send_messages) &&
            mln::utility::check_permissions(url_guild, url_channel, cmd_data.cmd_bot, {dpp::permissions::p_view_channel, dpp::permissions::p_read_message_history});
        //Return an error if the bot is not allowed to send messages in the dump channel or if it can't access the url message
        if (!retrieve_create_msg_permission) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed command, the bot doesn't have the permission to either retrieve the original msg or to send messages to the dump channel!", {true, dpp::loglevel::ll_debug});
            co_return;
        }

        //If we didn't find a cache result we retrieve the original msg, then we create a new storage msg that we will link to in the database
        const dpp::confirmation_callback_t msg_retrieve_result = co_await delta()->bot.co_message_get(url_data.message_id, url_data.channel_id);

        //Return an error if we couldn't retrieve the original message
        if (msg_retrieve_result.is_error()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
                "Failed to retrieve message described by the given url! Error: " + msg_retrieve_result.get_error().human_readable + ".");
            co_return;
        }

        dpp::message original_msg = std::move(msg_retrieve_result.get<dpp::message>());
        
        if (original_msg.tts) {
            //Return error if the message is tts and the bot doesn't have the permission to send it to dump channel
            if (!mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_send_tts_messages)) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed command, the bot doesn't have the permission to send tts messages to the dump channel!", {true, dpp::loglevel::ll_debug});
                co_return;
            }
        }

        //Iterate through all available attachments, download them and attach them to message to store in dump channel. Check permission first
        std::vector<std::tuple<dpp::http_request_completion_t, std::string, std::string>> download_results{};
        download_results.reserve(original_msg.attachments.size());

        if (original_msg.attachments.size() != 0) {
            if (!mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.dump_channel, cmd_data.cmd_bot, dpp::permissions::p_attach_files)) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed command, the bot doesn't have the permission to upload attachments in the dump channel!");
                co_return;
            }
        }

        for (const dpp::attachment& attach : original_msg.attachments) {
            //The attachment is not valid, return an error
            if (attach.owner == nullptr || attach.owner->owner == nullptr ||
                attach.id == 0 || attach.url.empty()) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed to find valid attachment while inserting an url to database!");
                co_return;
            }

            //Download attachments and add them to list of downloaded attachments, return an error if operation fails
            dpp::http_request_completion_t download_res = co_await delta()->bot.co_request(attach.url, dpp::http_method::m_get, "", attach.content_type);
            if (download_res.status != 200) {
                co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                    "Failed to download valid attachment while inserting an url to database! Error: "
                    + std::to_string(download_res.error) + ", status: " + std::to_string(download_res.status));
                co_return;
            }

            download_results.emplace_back(std::make_tuple(std::move(download_res), attach.filename, attach.content_type));
        }

        //Attach data to message
        original_msg.attachments.clear();
        for (const std::tuple<dpp::http_request_completion_t, std::string, std::string>& http_res_tuple : download_results) {
            original_msg.add_file(std::get<1>(http_res_tuple), std::get<0>(http_res_tuple).body, std::get<2>(http_res_tuple));
        }

        //Store the retrieved message in the dump channel
        const dpp::confirmation_callback_t msg_create_result = co_await delta()->bot.co_message_create(
            original_msg.set_channel_id(cmd_data.dump_channel->id).set_guild_id(cmd_data.cmd_guild->id));

        //Return an error if we failed to create the storage message
        if (msg_create_result.is_error()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to create message in the designated dump channel! Error: " + msg_create_result.get_error().human_readable + ".");
            co_return;
        }

        created_msg = std::move(msg_create_result.get<dpp::message>());
        final_url = created_msg.get_url();
    }
    
    //Execute the query using the storage msg we created (or retrieved from cache) for the stored url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, final_url, thinking);

    if (!is_query_success) {
        //If the query failed make sure to delete the message we created for storage (if present)
        if (created_msg.id != 0) {
            //We don't need to check for permissions here, the created message was created by the bot. No need for perms to delete your own message
            dpp::confirmation_callback_t delete_res = co_await delta()->bot.co_message_delete(created_msg.id, cmd_data.dump_channel->id);
            if (delete_res.is_error()) {
                //We don't do edit_response because exec_query is already handling that
                delta()->bot.log(dpp::loglevel::ll_error, "Failed to delete the created msg! Error: " + delete_res.get_error().human_readable);
            }
        }

        co_return;
    }

    //Update caches
    if (created_msg.id != 0) {
        delta()->messages_cache.add_element(created_msg.id, std::move(created_msg));
    }
    delta()->show_all_cache.remove_element(cmd_data.cmd_guild->id);
    delta()->show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    //Return a success reply to the user
    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
        "Database operation successful!", {false, dpp::loglevel::ll_debug});
}
dpp::task<bool> mln::db_insert::execute_query(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& current_event, const db_cmd_data_t& cmd_data, const std::string& url, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Retrieve remaining data required for the database query
    std::string description;
    const dpp::command_value desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        description = std::get<std::string>(desc_param);
    }
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    //Bind query parameters
    mln::db_result res1 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_user, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    mln::db_result res3 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_url, url, mln::db_text_encoding::utf8);
    mln::db_result res4 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name, mln::db_text_encoding::utf8);
    mln::db_result res5;
    if (valid_description) {
        res5 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc, description, mln::db_text_encoding::utf8);
    } else {
        res5 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc);
    }

    //Check if any error occurred in the binding process, in case return an error
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok || res5 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, current_event, delta()->bot, "Failed to bind query parameters, internal error!");
        co_return false;
    }

    //Prepare callbacks for query execution
    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    //Execute query and return an error if the query failed or if no element was added
    mln::db_result res = delta()->db.exec(data.saved_stmt, calls);
    if (res != mln::db_result::ok || !db_success) {
        co_await mln::utility::co_conclude_thinking_response(thinking, current_event, delta()->bot, res == mln::db_result::ok && !db_success ?
            "Failed while executing database query!" " The given name was already taken by another record in the database!" :
            "Failed while executing database query! Internal error!");
        co_return false;
    }
    
    co_return true;
}
