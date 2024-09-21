#include "commands/slash/db/db_insert.h"
#include "database/database_handler.h"
#include "utility/utility.h"
#include "utility/constants.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "utility/response.h"
#include "utility/json_err.h"
#include "utility/http_err.h"
#include "utility/reply_log_data.h"

#include <dpp/guild.h>
#include <dpp/channel.h>
#include <dpp/dispatcher.h>
#include <dpp/permissions.h>
#include <dpp/cluster.h>
#include <dpp/utility.h>

const std::unordered_map<mln::db_command_type, std::tuple<
    mln::db_init_type_flag,
    std::function<dpp::task<void>(const mln::db_insert&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&, bool)>>>
    mln::db_insert::s_mapped_commands_info{

    {mln::db_command_type::url, {db_init_type_flag::cmd_data | db_init_type_flag::dump_channel | db_init_type_flag::thinking, &mln::db_insert::command_url}},
    {mln::db_command_type::text, {db_init_type_flag::cmd_data | db_init_type_flag::dump_channel, &mln::db_insert::command_text}},
    {mln::db_command_type::file, {db_init_type_flag::cmd_data | db_init_type_flag::dump_channel | db_init_type_flag::thinking, &mln::db_insert::command_file}},
    {mln::db_command_type::help, {mln::db_init_type_flag::none, &mln::db_insert::command_help}},
};

mln::db_insert::db_insert(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, data{ .valid_stmt = true }, db{ in_db } {

    const mln::db_result_t res1 = db.save_statement(
        "INSERT OR ABORT INTO storage(guild_id, name, url, desc, user_id, nsfw) VALUES(:GGG, :NNN, :LLL, :DDD, :UUU, :WWW) RETURNING user_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert url stmt! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        data.valid_stmt = false;
    } else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data.saved_stmt, 0, ":LLL", data.saved_param_url);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        const mln::db_result_t res15 = db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_desc);
        const mln::db_result_t res16 = db.get_bind_parameter_index(data.saved_stmt, 0, ":WWW", data.saved_param_nsfw);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok || res15.type != mln::db_result::ok || res16.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert url stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], url_param: [{}, {}], user_param: [{}, {}], desc_param: [{}, {}], nsfw_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text,
                mln::database_handler::get_name_from_result(res15.type), res15.err_text,
                mln::database_handler::get_name_from_result(res16.type), res16.err_text));
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_insert::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const {

    //Find the command variant and execute it. If no valid command variant found return an error
    const bool is_first_reply = (mln::db_insert::get_requested_initialization_type(type) & mln::db_init_type_flag::thinking) == mln::db_init_type_flag::none;
    const auto it_func = s_mapped_commands_info.find(type);
    if (it_func == s_mapped_commands_info.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed command, the given sub_command is not supported!"), bot(), &event_data, 
            std::format("Failed command, the given sub_command [{}] is not supported for /db insert!", mln::get_cmd_type_text(type)));
        co_return;
    }

    //If the query statement was not saved correctly, return an error
    if (!data.valid_stmt) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed database operation, the database was not initialized correctly!"), bot(), &event_data, 
            std::format("Failed database /db insert operation [{}], the database was not initialized correctly!", mln::get_cmd_type_text(type)));
        co_return;
    }

    //Checking nsfw permission on dump channel
    const dpp::command_value nsfw_param = event_data.get_parameter("nsfw");
    const bool nsfw = std::holds_alternative<bool>(nsfw_param) ? std::get<bool>(nsfw_param) : false;
    if (nsfw && cmd_data.dump_channel) {
        if (!cmd_data.dump_channel->is_nsfw()) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
                "Failed command, the dump channel needs to be nsfw in order to insert nsfw records in the database!"), bot());
            co_return;
        }
    }

    co_await std::get<1>(it_func->second)(*this, event_data, cmd_data, nsfw);
}

mln::db_init_type_flag mln::db_insert::get_requested_initialization_type(const db_command_type cmd) const {

    const auto it = s_mapped_commands_info.find(cmd);
    if (it == s_mapped_commands_info.end()) {
        return mln::db_init_type_flag::all;
    }
    return std::get<0>(it->second);
}

dpp::task<void> mln::db_insert::command_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const {
    //Verify if given url is a valid discord message url
    const std::string input_url = std::get<std::string>(event_data.get_parameter("url"));
    uint64_t url_guild_id{0}, url_channel_id{0}, url_message_id{0};
    const bool is_url_msg = mln::utility::extract_message_url_data(input_url, url_guild_id, url_channel_id, url_message_id);

    if (!is_url_msg) {
        //Verify if the given url is a valid discord attachment url
        std::string name{};
        const bool is_url_attach = mln::utility::extract_generic_attachment_url_data(input_url, url_guild_id, url_channel_id, name);

        //The given url is neither a message url nor an attachment url, return an error
        if (!is_url_attach) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed url parsing, the given url is neither pointing to a valid message nor to a valid attachment!"), bot(), &event_data, 
                std::format("Failed url parsing, the given url [{}] is neither pointing to a valid message nor to a valid attachment!", input_url));
            co_return;
        }

        //Handle attachment url
        co_await manage_attach_url(event_data, cmd_data, {input_url, name}, nsfw);
    } else {
        msg_url_t url_data{.guild_id = url_guild_id, .channel_id = url_channel_id, .message_id = url_message_id};
        //Handle msg url
        co_await manage_msg_url(event_data, cmd_data, url_data, nsfw);
    }
}
dpp::task<void> mln::db_insert::command_text(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const {
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

    //If the bot cannot send messages in the dump channel don't bother, return an error
    const bool create_msg_permission = mln::perms::check_permissions(cmd_data.dump_channel_bot_perm, dpp::permissions::p_send_messages | dpp::permissions::p_view_channel);
    if (!create_msg_permission) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, event_data,
            "Failed command, the bot doesn't have permission to write into the dump channel!"), bot());
        co_return;
    }

    //We get a copy of the 'standard' modal, and give it an unique id
    dpp::interaction_modal_response modal{s_modal};
    const std::string modal_id = std::format("dbin_{}", static_cast<uint64_t>(event_data.command.id));
    modal.set_custom_id(modal_id);

    //Try to reply with a dialog box, return an error if failure
    const dpp::confirmation_callback_t dialog_conf = co_await event_data.co_dialog(modal);
    if (dialog_conf.is_error()) {
        const dpp::error_info err = dialog_conf.get_error();

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, event_data,
            "Failed to create dialog box!"), bot(), &event_data, 
            std::format("Failed to create dialog box for insert command! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    const dpp::confirmation dialog_conf_data = dialog_conf.get<dpp::confirmation>();
    if (!dialog_conf_data.success) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "An internal error occurred!"), bot(), &event_data, 
            "The co_dialog confirmation result is false for insert command!");
        co_return;
    }
    
    mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
        "Dialog menu opened, waiting for the user's input!"), bot());

    //Wait for a max amount of time for the dialog submission
    const auto &result = co_await dpp::when_any{
        bot().co_sleep(s_max_time),
        bot().on_form_submit.when([&modal_id](const dpp::form_submit_t& event_data) {return event_data.custom_id == modal_id;})};

    //If the timer run out return an error
    if (result.index() == 0) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to insert text data in time, command interrupted!"), bot());
        co_return;
    }

    //If an exception occurred return an error
    if (result.is_exception()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to insert text data, unknown error occurred!"), bot(), &event_data, 
            "Failed to insert text data, unknown error occurred!");
        co_return;
    }

    //It was suggested to copy the event from documentation of ::when
    dpp::form_submit_t form_data = result.get<1>();
    if (form_data.cancelled) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, form_data,
            "Failed to insert text data! Internal error!"), bot(), &form_data, "Failed to insert text data! Event cancelled!");
        co_return;
    }

    dpp::async<dpp::confirmation_callback_t> thinking = form_data.co_thinking(true);
    //Get parameters, make sure there is at least 1 character. Remember the parameters are optional!
    dpp::message msg = dpp::message{}.set_guild_id(cmd_data.cmd_guild->id).set_channel_id(cmd_data.dump_channel->id);
    dpp::embed embed{};
    for (const dpp::component& component : form_data.components) {
        if (component.components.size() == 0) {
            continue;
        }
        const dpp::component field = component.components[0];

        if (!std::holds_alternative<std::string>(field.value) || std::get<std::string>(field.value).size() == 0) {
            continue;
        }

        if (field.custom_id == "0") {
            msg.set_content(std::get<std::string>(field.value));
        } else if (field.custom_id == "1") {
            embed.set_description(std::get<std::string>(field.value));
        } else if (field.custom_id == "2" || field.custom_id == "3") {
            embed.add_field("", std::get<std::string>(field.value));
        }
    }

    //Add the embed to the msg if something has been written into it
    if (embed.description.size() > 0 || embed.fields.size() > 0) {
        msg.add_embed(embed);
    } else {
        //If embeds are empty AND the msg content is empty return an error
        if (msg.content.size() == 0) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, form_data,
                "Failed command, there needs to be at least 1 character in total in the overall provided text!"), bot());
            co_return;
        }
    }

    mln::utility::conf_callback_is_error(co_await thinking, bot(), nullptr, "Failed dialog thinking for insert text command!");

    //Return an error if we fail to send the message
    const dpp::confirmation_callback_t create_msg_conf = co_await bot().co_message_create(msg);
    if (create_msg_conf.is_error()) {
        const dpp::error_info err = create_msg_conf.get_error();

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, form_data,
            "Failed to create message in the dump channel!"), bot(), &form_data, 
            std::format("Failed to create message in the dump channel! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    //Execute the database query using the created message url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, form_data, cmd_data, 
        dpp::utility::message_url(cmd_data.cmd_guild->id, cmd_data.dump_channel->id, std::get<dpp::message>(create_msg_conf.value).id), nsfw);
    //Errors are handled by execute_query
    if (!is_query_success) {
        //If the query failed make sure to delete the message we created for storage (if present)
        if (std::get<dpp::message>(create_msg_conf.value).id != 0) {
            //We don't need to check for permissions here, the created message was created by the bot. No need for perms to delete your own message
            mln::utility::conf_callback_is_error(co_await bot().co_message_delete(std::get<dpp::message>(create_msg_conf.value).id, cmd_data.dump_channel->id), bot(), &form_data, "Failed to delete the created msg!");
        }

        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, form_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(form_data, bot(), "Failed insert command conclusion reply!");
    }
}
dpp::task<void> mln::db_insert::command_file(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool nsfw) const {
    //Find given file from resolved elements, if not found return an error
    const dpp::snowflake file_id = std::get<dpp::snowflake>(event_data.get_parameter("file"));
    const auto it = event_data.command.resolved.attachments.find(file_id);
    if (it == event_data.command.resolved.attachments.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
            "Failed database operation, failed to retrieve given file!"), bot(), &event_data, 
            "Failed insert database operation, failed to retrieve given file!");
        co_return;
    }

    //Verify bot permission
    if (!mln::perms::check_permissions(cmd_data.dump_channel_bot_perm, 
        dpp::permissions::p_attach_files | dpp::permissions::p_send_messages | dpp::permissions::p_view_channel)) {

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
            "Failed command, the bot doesn't have the permission to upload attachments or send messages in the dump channel!"), bot());

        co_return;
    }

    //Download attachment, return error on failure
    const dpp::http_request_completion_t download_res = co_await bot().co_request(it->second.url, dpp::http_method::m_get, "", it->second.content_type, {}, "1.1", mln::constants::get_big_files_request_timeout());
    if (download_res.status != 200) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
            "Failed database operation, failed to download given file!"), bot(), &event_data, 
            std::format("Failed database operation, failed to download given file! Status: [{}], error: [{}]", mln::get_http_err_text(download_res.status), mln::get_dpp_http_err_text(download_res.error)));

        co_return;
    }

    //Create and send message into dump channel, return error on fail
    dpp::message message_to_store = dpp::message{}
        .add_file(it->second.filename, download_res.body, it->second.content_type)
        .set_channel_id(cmd_data.dump_channel->id)
        .set_guild_id(cmd_data.cmd_guild->id);

    const dpp::confirmation_callback_t create_res = co_await bot().co_message_create(message_to_store);
    if (create_res.is_error()) {
        const dpp::error_info err = create_res.get_error();

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
            "Failed database operation, failed to send file to dump channel!"), bot(), &event_data, 
            std::format("Failed database operation, failed to send file to dump channel! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));

        co_return;
    }

    if (!co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, 
        dpp::utility::message_url(cmd_data.cmd_guild->id, cmd_data.dump_channel->id, std::get<dpp::message>(create_res.value).id), nsfw)) {
        //If the query failed make sure to delete the message we created for storage (if present)
        if (std::get<dpp::message>(create_res.value).id != 0) {
            //We don't need to check for permissions here, the created message was created by the bot. No need for perms to delete your own message
            mln::utility::conf_callback_is_error(co_await bot().co_message_delete(std::get<dpp::message>(create_res.value).id, cmd_data.dump_channel->id), bot(), &event_data, "Failed to delete the created msg!");
        }

        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed insert command conclusion reply!");
    }
}
dpp::task<void> mln::db_insert::command_help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, bool) const {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db insert` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db insert` set of commands is used to insert new records into the database. The new record will be identified by the name supplied by the user, and it will be retrievable through the command `/db select single chosen_name`.

Each record can be supplied with a description (max 100 characters), which will allow the user and other server members to quickly understand what is stored in the record while using the retrieval commands `/db select` and `/db show`.

Attempting to insert a new record using a name that is already being used by another record will result in an error.

If a dump channel has been set for the current server, all the stored records will be kept in the dump channel for storage purposes.

If the new record is tagged with nsfw = True its contents will be displayed only in nsfw channels.

Currently only a total of roughly 200 MB size for the attachments is supported, attempting to store a bigger total file size will likely result in an error.

Only ASCII printable characters are accepted as input for the `name` and `description` parameters.

**Types of insert:**

- **/db insert text**  
  *Parameters:* name[text, required], nsfw[boolean, required], description[text, optional].  
  This command will ask you to provide a name for the new record and, optionally, a description for it.  
  Once that is done, a dialog menu will open, allowing the user to insert up to 8000 characters in text form to be stored in the database. The dialog menu contains 4 fields with varying character limits that can be filled.  
  The user has up to 5 minutes to insert the data and confirm it. If time runs out or if the menu is dismissed, the command will terminate.  
  Once the text is submitted (and no error occurs), it will be stored in the dump channel and will be available for retrieval.

- **/db insert file**  
  *Parameters:* name[text, required], file[attachment, required], nsfw[boolean, required], description[text, optional].  
  This command will ask you to provide a name for the new record, an attachment, and, optionally, a description for it.  
  Once that is done, the bot will attempt to create a new record and save the file in the dump channel. Once all is done (and no error occurs), the new record containing the given file will be stored and available for retrieval.

- **/db insert url**  
  *Parameters:* name[text, required], url[text, required], nsfw[boolean, required], description[text, optional].  
  This command will ask you to provide a name for the new record, a URL link to a Discord message/attachment, and, optionally, a description for it.  
  The accepted URLs have the following forms:  
  - `https://discord.com/channels/[number]/[number]/[number]` (for links to Discord messages).  
  - `https://cdn.discordapp.com/attachments/[number]/[number]/[filename]...` (for links to Discord attachments).  
  - `https://cdn.discordapp.com/ephemeral-attachments/[number]/[number]/[filename]...` (for links to Discord ephemeral attachments).  
  If the supplied URL is valid and no error occurs, the message/attachment will be saved in the dump channel and will be ready for retrieval.  
  This is the most versatile and powerful command in the `insert` category since it allows the user to store anything (even from other servers) as long as the bot has access to it. Using this command allows the user to store messages with multiple attachments and text content (something that cannot be done by the other two `insert` variants).)"""));

    if (mln::utility::conf_callback_is_error(co_await event_data.co_reply(s_info), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the db insert help text!");
    }

    co_return;
}
dpp::task<void> mln::db_insert::manage_attach_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const std::tuple<std::string, std::string>& url_name, bool nsfw) const {
    const bool create_msg_permission = mln::perms::check_permissions(cmd_data.dump_channel_bot_perm, 
        dpp::permissions::p_send_messages | dpp::permissions::p_view_channel | dpp::permissions::p_attach_files);
    //Return an error if the bot is not allowed to send messages in the dump channel
    if (!create_msg_permission) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, 
            "Failed command, the bot doesn't have the permission to send messages in the dump channel!"), bot());
        co_return;
    }

    //Download the url first, then attach to msg and send to dump
    const dpp::http_request_completion_t downloaded_attach = co_await bot().co_request(std::get<0>(url_name), dpp::http_method::m_get, "", "text/plain", {}, "1.1", mln::constants::get_big_files_request_timeout());
    if (downloaded_attach.status != 200) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to download the given attachment!"), bot(), &event_data, 
            std::format("Failed to download the given attachment! Status: [{}], error: [{}].", mln::get_http_err_text(downloaded_attach.status), mln::get_dpp_http_err_text(downloaded_attach.error)));
        co_return;
    }

    //Make an API request to create a message in the dump channel with the given attachment url
    const dpp::confirmation_callback_t msg_create_result = 
        co_await bot().co_message_create(dpp::message{}
            .set_channel_id(cmd_data.dump_channel->id)
            .set_guild_id(cmd_data.cmd_guild->id)
            .add_file(std::get<1>(url_name), downloaded_attach.body));

    //If message creation failed return an error
    if (msg_create_result.is_error()) {
        const dpp::error_info err = msg_create_result.get_error();
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to create message in the designated dump channel!"), bot(), &event_data,
            std::format("Failed to create message in the designated dump channel! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    //Execute the database query using the created message url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, 
        dpp::utility::message_url(cmd_data.cmd_guild->id, cmd_data.dump_channel->id, std::get<dpp::message>(msg_create_result.value).id), nsfw);
    //Errors are handled by execute_query
    if (!is_query_success) {
        //If the query failed make sure to delete the message we created for storage (if present)
        if (std::get<dpp::message>(msg_create_result.value).id != 0) {
            //We don't need to check for permissions here, the created message was created by the bot. No need for perms to delete your own message
            mln::utility::conf_callback_is_error(co_await bot().co_message_delete(std::get<dpp::message>(msg_create_result.value).id, cmd_data.dump_channel->id), bot(), &event_data, "Failed to delete the created msg!");
        }

        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    //Return a success reply to the user
    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed insert command conclusion reply!");
    }
}
dpp::task<void> mln::db_insert::manage_msg_url(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const mln::msg_url_t& url_data, bool nsfw) const {
    
    //Check permission for storing msg
    const bool create_msg_permission =
        mln::perms::check_permissions(cmd_data.dump_channel_bot_perm, dpp::permissions::p_send_messages | dpp::permissions::p_view_channel);
    //Return an error if the bot is not allowed to send messages in the dump channel or if it can't access the url message
    if (!create_msg_permission) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed command, the bot doesn't have the permission to send messages in the dump channel!"), bot());
        co_return;
    }

    //Retrieve url guild data
    const mln::reply_log_data_t reply_log_data{ &event_data, &bot(), false };
    std::shared_ptr<const dpp::guild> url_guild;
    if (url_data.guild_id == cmd_data.cmd_guild->id && cmd_data.cmd_guild) {
        url_guild = cmd_data.cmd_guild;
    } else {
        const std::optional<std::shared_ptr<const dpp::guild>> url_guild_opt = co_await mln::caches::get_guild_full(url_data.guild_id, reply_log_data);
        if (!url_guild_opt.has_value()) {
            co_return;
        }
        url_guild = url_guild_opt.value();
    }

    //Retrieve url channel data
    std::shared_ptr<const dpp::channel> url_channel;
    if (url_data.channel_id == cmd_data.cmd_channel->id && cmd_data.cmd_channel) {
        url_channel = cmd_data.cmd_channel;
    } else {
        const std::optional<std::shared_ptr<const dpp::channel>> url_channel_opt = co_await mln::caches::get_channel_full(url_data.channel_id, reply_log_data);
        if (!url_channel_opt.has_value()) {
            co_return;
        }
        url_channel = url_channel_opt.value();
    }

    //Retrieve perms for url msg channel
    const std::optional<dpp::permission> url_bot_perm_opt = co_await mln::perms::get_computed_permission_full(*(url_guild), *(url_channel), *(cmd_data.cmd_bot), reply_log_data);
    if (!url_bot_perm_opt.has_value()) {
        co_return;
    }

    const bool retrieve_msg_permission =
        mln::perms::check_permissions(url_bot_perm_opt.value(), dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history);
    //Return an error if the bot is not allowed to access the url message
    if (!retrieve_msg_permission) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed command, the bot doesn't have the permission to retrieve the original msg!"), bot());
        co_return;
    }

    //Retrieve message from cache (if present) using the given url message id value, otherwise ask discord api
    std::optional<dpp::message> opt_msg = co_await mln::caches::get_message_full(url_data.message_id, url_data.channel_id, reply_log_data);
    if (!opt_msg.has_value()) {
        co_return;
    }

    //Check other perms to check to store msg to dump channel
    const dpp::permission to_check = co_await mln::perms::get_additional_perms_required(opt_msg.value(), bot(), cmd_data.cmd_guild->id);

    if (to_check != 0) {
        if (!mln::perms::check_permissions(cmd_data.dump_channel_bot_perm, to_check)) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed command, the bot doesn't have the additional permissions to send a msg to dump channel!"), bot());
            co_return;
        }
    }

    //Verify if the attachments are already stored as file_data and, if not, iterate through all available attachments, download them and attach them to message to store in dump channel.
    if (opt_msg.value().attachments.size() != opt_msg.value().file_data.size()) {
        std::vector<std::tuple<dpp::http_request_completion_t, std::string, std::string>> download_results{};
        download_results.reserve(opt_msg.value().attachments.size());

        for (const dpp::attachment& attach : opt_msg.value().attachments) {
            //The attachment is not valid, return an error
            if (attach.owner == nullptr || attach.owner->owner == nullptr ||
                attach.id == 0 || attach.url.empty()) {

                mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                    "Failed to find valid attachment while inserting an url to database!"), bot(), &event_data, 
                    std::format("Failed to find valid attachment while inserting an url to database! Owner: [{}], owner->owner: [{}], id: [{}], url: [{}].", 
                        (attach.owner ? "not null" : "null"), (attach.owner ? (attach.owner->owner ? "not null" : "null") : "null"), static_cast<uint64_t>(attach.id), attach.url));
                co_return;
            }

            //Download attachments and add them to list of downloaded attachments, return an error if operation fails
            const dpp::http_request_completion_t download_res = co_await bot().co_request(attach.url, dpp::http_method::m_get, "", attach.content_type, {}, "1.1", mln::constants::get_big_files_request_timeout());
            if (download_res.status != 200) {
                mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                    "Failed to download valid attachment while inserting an url to database!"), bot(), &event_data,
                    std::format("Failed to download valid attachment while inserting an url to database! Status: [{}], error: [{}]", mln::get_http_err_text(download_res.status), mln::get_dpp_http_err_text(download_res.error)));
                co_return;
            }

            download_results.emplace_back(std::make_tuple(std::move(download_res), attach.filename, attach.content_type));
        }

        //Attach data to message
        opt_msg.value().attachments.clear();
        opt_msg.value().file_data.clear();
        for (const std::tuple<dpp::http_request_completion_t, std::string, std::string>& http_res_tuple : download_results) {
            opt_msg.value().add_file(std::get<1>(http_res_tuple), std::get<0>(http_res_tuple).body, std::get<2>(http_res_tuple));
        }
    }

    //Store the retrieved message in the dump channel
    const dpp::confirmation_callback_t msg_create_result = co_await bot().co_message_create(
        opt_msg.value().set_channel_id(cmd_data.dump_channel->id).set_guild_id(cmd_data.cmd_guild->id));

    //Return an error if we failed to create the storage message
    if (msg_create_result.is_error()) {
        const dpp::error_info err = msg_create_result.get_error();
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to create message in the designated dump channel!"), bot(), &event_data,
            std::format("Failed to create message in the designated dump channel! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable));
        co_return;
    }

    //NOTE: the dpp::message::get_url() is sometime bugged when used on a message received from a callback, the guild_id is set to 0 which makes the method return an empty url string
    //Execute the query using the storage msg we created (or retrieved from cache) for the stored url
    const bool is_query_success = co_await mln::db_insert::execute_query(event_data, event_data, cmd_data, 
        dpp::utility::message_url(cmd_data.cmd_guild->id, cmd_data.dump_channel->id, std::get<dpp::message>(msg_create_result.value).id), nsfw);
    if (!is_query_success) {
        //If the query failed make sure to delete the message we created for storage (if present)
        if (std::get<dpp::message>(msg_create_result.value).id != 0) {
            //We don't need to check for permissions here, the created message was created by the bot. No need for perms to delete your own message
            mln::utility::conf_callback_is_error(co_await bot().co_message_delete(std::get<dpp::message>(msg_create_result.value).id, cmd_data.dump_channel->id), bot(), &event_data, "Failed to delete the created msg!");
        }

        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id});

    //Return a success reply to the user
    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed insert command conclusion reply!");
    }
}

dpp::task<bool> mln::db_insert::execute_query(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& current_event, const db_cmd_data_t& cmd_data, const std::string& url, bool nsfw) const {
    if (url.empty()) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, current_event, "Failed to bind query parameters, given url is empty!"), bot(), &event_data, 
            "Failed to bind query parameters, given url is empty!");
        co_return false;
    }
    
    //Retrieve remaining data required for the database query
    std::string description;
    const dpp::command_value desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        description = std::get<std::string>(desc_param);

        if (!mln::utility::is_ascii_printable(description)) {
            mln::utility::conf_callback_is_error(
                co_await mln::response::make_response(false, current_event, "Failed to bind query parameters, given description is composed of invalid characters! Only ASCII printable characters are accepted [32,126]"), bot());
            co_return false;
        }
    }
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));
    if (!mln::utility::is_ascii_printable(name)) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, current_event, "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]"), bot());
        co_return false;
    }

    //Bind query parameters
    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_user, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    const mln::db_result_t res3 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_url, url, mln::db_text_encoding::utf8);
    const mln::db_result_t res4 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name, mln::db_text_encoding::utf8);
    mln::db_result_t res5;
    if (valid_description) {
        res5 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc, description, mln::db_text_encoding::utf8);
    } else {
        res5 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc);
    }
    const mln::db_result_t res6 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_nsfw, static_cast<int>(nsfw));

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok || res4.type != mln::db_result::ok || res5.type != mln::db_result::ok || res6.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, current_event, "Failed to bind query parameters, internal error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], user_param: [{}, {}], url_param: [{}, {}], name_param: [{}, {}], desc_param: [{}, {}], nsfw_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text,
                mln::database_handler::get_name_from_result(res4.type), res4.err_text,
                mln::database_handler::get_name_from_result(res5.type), res5.err_text,
                mln::database_handler::get_name_from_result(res6.type), res6.err_text));
        co_return false;
    }

    //Prepare callbacks for query execution
    bool db_success = false;
    const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    //Execute query and return an error if the query failed or if no element was added
    const mln::db_result_t res = db.exec(data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !db_success) {
        const std::string err_text = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !db_success ?
            "Failed while executing database query! The given name was already taken by another record in the database!" :
            std::format("Failed while executing database query! Internal error! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);

        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, current_event, "Failed while executing database query! Internal error!"), bot(), &event_data, err_text);
        co_return false;
    }

    co_return true;
}
