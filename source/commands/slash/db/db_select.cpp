#include "commands/slash/db/db_select.h"
#include "bot_delta.h"
#include "database/database_handler.h"
#include "utility/utility.h"

mln::db_select::db_select(bot_delta* const delta) : base_db_command(delta), data{.valid_stmt = true} {

    mln::db_result res1 = delta->db.save_statement("SELECT url FROM storage WHERE guild_id = :GGG AND name = :NNN;", data.saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save select verbose stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_select::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const std::unordered_map<mln::db_command_type, std::function<dpp::task<void>(mln::db_select*, const dpp::slashcommand_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>> s_allowed_subcommands{
        {mln::db_command_type::single, &mln::db_select::select},
        {mln::db_command_type::help, &mln::db_select::help},
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

mln::db_init_type_flag mln::db_select::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::single, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::help, db_init_type_flag::cmd_data},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}

dpp::task<void> mln::db_select::select(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Retrieve remaining data required for the database query
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    //Bind query parameters
    mln::db_result res1 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name, mln::db_text_encoding::utf8);

    //Check if any error occurred in the binding process, in case return an error
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to bind query parameters, internal error!");
        co_return;
    }

    //Prepare callbacks for query execution
    std::string url{};
    mln::database_callbacks_t calls{};
    calls.callback_data = static_cast<void*>(&url);
    calls.type_definer_callback = [](void*, int) {return true;};
    calls.data_adder_callback = [](void* s_ptr, int, mln::db_column_data_t&& d) {
        std::string* s = static_cast<std::string*>(s_ptr);
        *s = reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data)));
    };

    //Execute query and return an error if the query failed or if no element was added
    mln::db_result res = delta()->db.exec(data.saved_stmt, calls);
    if (res != mln::db_result::ok || url.empty()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, res == mln::db_result::ok && url.empty() ?
            "Failed while executing database query!" " The given name was not found in the database!" :
            "Failed while executing database query! Internal error!");
        co_return;
    }

    //Extract url values, return error on fail
    uint64_t url_guild_id{0}, url_channel_id{0}, url_message_id{0};
    if (!mln::utility::extract_message_url_data(url, url_guild_id, url_channel_id, url_message_id)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
            "Failed command, internal error! The url extracted from the database is not a valid url!");
        co_return;
    }

    std::optional<dpp::message> cached_msg = delta()->messages_cache.get_element(url_message_id);
    const bool use_cached_msg = cached_msg.has_value();
    dpp::message original_msg{};
    if (use_cached_msg) {
        original_msg = cached_msg.value();
    } else {
        //Retrieve url guild data
        const std::tuple<dpp::guild*, dpp::guild> url_guild_pair = co_await mln::utility::get_guild(url_guild_id, delta()->bot);
        const dpp::guild* url_guild = std::get<0>(url_guild_pair);
        if (url_guild == nullptr) {
            //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
            url_guild = &std::get<1>(url_guild_pair);
        }

        //Retrieve url channel data
        const std::tuple<dpp::channel*, dpp::channel> url_channel_pair = co_await mln::utility::get_channel(event_data, url_channel_id, delta()->bot);
        const dpp::channel* url_channel = std::get<0>(url_channel_pair);
        if (url_channel == nullptr) {
            //Make sure this pointer is no longer used when this function ends. Make sure to co_await the manage_... functions at the end
            url_channel = &std::get<1>(url_channel_pair);
        }

        //If we failed to find the guild or the channel, we return an error
        if (url_channel->id == 0 || url_guild->id == 0) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, impossible to retrieve url guild and url channel data!");
            co_return;
        }

        //Return error if the bot doesn't have the required permissions
        if (!mln::utility::check_permissions(url_guild, url_channel, cmd_data.cmd_bot, {dpp::permissions::p_view_channel, dpp::permissions::p_read_message_history})) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, the bot doesn't have the permissions to retrieve the message!");
            co_return;
        }

        //Retrieve message, return error on fail
        dpp::confirmation_callback_t retrieve_result = co_await delta()->bot.co_message_get(url_message_id, url_channel->id);
        if (retrieve_result.is_error()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed command, the bot couldn't retrieve the requested message! Error: " + retrieve_result.get_error().human_readable);
            co_return;
        }

        original_msg = retrieve_result.get<dpp::message>();
        delta()->messages_cache.add_element(original_msg.id, std::move(retrieve_result.get<dpp::message>()));
    }

    //Download the attachments if needed
    std::vector<std::tuple<dpp::http_request_completion_t, std::string, std::string>> download_results{};
    download_results.reserve(original_msg.attachments.size());
    for (const dpp::attachment& attach : original_msg.attachments) {
        if (attach.owner == nullptr ||
            attach.id == 0 || attach.url.empty()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to find valid attachment while selecting the message from database!");
            co_return;
        }

        //Download attachments and add them to list of downloaded attachments, return an error if operation fails
        dpp::http_request_completion_t download_res = co_await delta()->bot.co_request(attach.url, dpp::http_method::m_get, "", attach.content_type);
        if (download_res.status != 200) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to download valid attachment while selecting the message from database! Error: "
                + std::to_string(download_res.error) + ", status: " + std::to_string(download_res.status));
            co_return;
        }

        download_results.emplace_back(std::make_tuple(std::move(download_res), attach.filename, attach.content_type));
    }

    //Add the downloaded attachments to final message, then set the target channel and guild
    if (download_results.size() > 0) {
        //Return an error if the bot doesn't have permission to attach files
        if (!mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.cmd_channel, cmd_data.cmd_bot, dpp::permissions::p_attach_files)) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, the bot doesn't have the permissions to send the message with attachments in the requested channel!");
            co_return;
        }

        original_msg.attachments.clear();
        for (const std::tuple<dpp::http_request_completion_t, std::string, std::string>& tup : download_results) {
            original_msg.add_file(std::get<1>(tup), std::get<0>(tup).body, std::get<2>(tup));
        }
    }
    original_msg.set_channel_id(cmd_data.cmd_channel->id).set_guild_id(cmd_data.cmd_guild->id);

    //Send the message to the target channel, depending on how 'broadcast' was set
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    
    if (broadcast) {
        //Return error if the bot doesn't have the required permissions to write in the command channel
        if (!mln::utility::check_permissions(cmd_data.cmd_guild, cmd_data.cmd_channel, cmd_data.cmd_bot, dpp::permissions::p_send_messages)) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed command, the bot doesn't have the permissions to send the message in the requested channel!");
            co_return;
        }

        const dpp::confirmation_callback_t send_result = co_await delta()->bot.co_message_create(original_msg);
        if (send_result.is_error()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
                "Failed command, the bot failed to send the message to the user's channel! Error: " + send_result.get_error().human_readable);
            co_return;
        }

        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Message retrieved!", {false, dpp::loglevel::ll_debug});

    } else {
        original_msg.set_flags(dpp::m_ephemeral);

        if (thinking.has_value()) {
            co_await thinking.value();
        }
        event_data.edit_response(original_msg);
    }
}

dpp::task<void> mln::db_select::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db select` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db select` set of commands is used to retrieve data from the database. By supplying a name, the bot will attempt to find a record with the same name and display the content. If the supplied name is not present in the database, an error will occur.

This is the main set of commands used to retrieve and display records inserted into the database.

**Types of select:**

- **/db select single**  
  *Parameters:* name[text, required], broadcast[boolean, optional].  
  This command asks for a record name and, optionally, a broadcast option. If broadcast is set to `false`, the result will only be shown to the user who invoked the command; if set to `true`, the result will be shown to everyone in the channel where the command was invoked.  
  If you have trouble remembering a record name, use the `/db show` commands to view all or some of the record names in the database, along with their descriptions (if present).)"""));

    event_data.reply(s_info);
    co_return;
}
