#include "commands/slash/db/db_select.h"
#include "database/database_handler.h"
#include "utility/utility.h"
#include "utility/caches.h"
#include "utility/perms.h"
#include "utility/constants.h"
#include "utility/response.h"
#include "utility/http_err.h"
#include "utility/json_err.h"
#include "utility/reply_log_data.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

const std::unordered_map<mln::db_command_type, std::tuple<
    mln::db_init_type_flag,
    std::function<dpp::task<void>(const mln::db_select&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>>
    mln::db_select::s_mapped_commands_info{

    {mln::db_command_type::single, {mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking, &mln::db_select::select}},
    {mln::db_command_type::help, {mln::db_init_type_flag::none, &mln::db_select::help}},
};

mln::db_select::db_select(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, data{ .valid_stmt = true }, db{ in_db } {

    const mln::db_result_t res = db.save_statement("SELECT url, nsfw FROM storage WHERE guild_id = :GGG AND name = :NNN;", data.saved_stmt);
    if (res.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save select stmt! Error: [{}], details: [{}].", 
            mln::database_handler::get_name_from_result(res.type), res.err_text));
        data.valid_stmt = false;
    } else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save select stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_select::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const {
    //Find the command variant and execute it. If no valid command variant found return an error
    const bool is_first_reply = (mln::db_select::get_requested_initialization_type(type) & mln::db_init_type_flag::thinking) == mln::db_init_type_flag::none;
    const auto it_func = s_mapped_commands_info.find(type);
    if (it_func == s_mapped_commands_info.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed command, the given sub_command is not supported!"), bot(), &event_data,
            std::format("Failed command, the given sub_command [{}] is not supported for /db select!", mln::get_cmd_type_text(type)));
        co_return;
    }

    //If the query statement was not saved correctly, return an error
    if (!data.valid_stmt) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed database operation, the database was not initialized correctly!"), bot(), &event_data,
            "Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    co_await std::get<1>(it_func->second)(*this, event_data, cmd_data);
}

mln::db_init_type_flag mln::db_select::get_requested_initialization_type(const db_command_type cmd) const {

    const auto it = s_mapped_commands_info.find(cmd);
    if (it == s_mapped_commands_info.end()) {
        return mln::db_init_type_flag::all;
    }
    return std::get<0>(it->second);
}

dpp::task<void> mln::db_select::select(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    //Check basic perms for sending result message to user channel
    if (!mln::perms::check_permissions(cmd_data.cmd_bot_perm, dpp::permissions::p_view_channel | dpp::permissions::p_send_messages)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed command, the bot doesn't have the required minimum perms to send messages in the user channel!"), bot());
        co_return;
    }
    
    //Retrieve remaining data required for the database query
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));
    if (!mln::utility::is_ascii_printable(name)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]"), bot());
        co_return;
    }

    //Bind query parameters
    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name, mln::db_text_encoding::utf8);

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, internal error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], name_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res1.type), res1.err_text, 
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    //Prepare callbacks for query execution
    std::tuple<std::string, bool> retrieved_data{};
    mln::database_callbacks_t calls{};
    calls.callback_data = static_cast<void*>(&retrieved_data);
    calls.type_definer_callback = [](void*, int) {return true;};
    calls.data_adder_callback = [](void* ptr, int c, mln::db_column_data_t&& d) {
        std::tuple<std::string, bool>* pair = static_cast<std::tuple<std::string, bool>*>(ptr);
        if (c == 0) {
            std::get<0>(*pair) = reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data)));
            return;
        }
        
        std::get<1>(*pair) = static_cast<bool>(std::get<int>(d.data));
    };

    //Execute query and return an error if the query failed or if no element was added
    const mln::db_result_t res = db.exec(data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || (std::get<0>(retrieved_data)).empty()) {
        const std::string err_text = !mln::database_handler::is_exec_error(res.type) && (std::get<0>(retrieved_data)).empty() ?
            std::format("Failed while executing database query! The given name was not found in the database! Name: [{}].", name) :
            std::format("Failed while executing database query! Internal error! Name: [{}], url: [{}], is_nsfw: [{}].", name, std::get<0>(retrieved_data), std::get<1>(retrieved_data));

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            err_text), bot(), &event_data,
            std::format("{} Error: [{}], details: [{}].", 
                err_text,
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    //If selected record is nsfw check if destination channel has the required nsfw tag
    if (std::get<1>(retrieved_data)) {
        if (!cmd_data.cmd_channel->is_nsfw()) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed command, the user channel needs to be nsfw in order to select nsfw records from the database!"), bot());
            co_return;
        }
    }

    //Extract url values, return error on fail
    uint64_t url_guild_id{0}, url_channel_id{0}, url_message_id{0};
    if (!mln::utility::extract_message_url_data((std::get<0>(retrieved_data)), url_guild_id, url_channel_id, url_message_id)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed command, internal error! The url extracted from the database is not a valid url!"), bot(), &event_data, 
            std::format("Failed command, internal error! The url extracted from the database is not a valid url! Url: [{}].", std::get<0>(retrieved_data)));
        co_return;
    }

    const mln::reply_log_data_t reply_log_data{ &event_data, &bot(), false };

    //Retrieve url guild data
    std::shared_ptr<const dpp::guild> url_guild;
    if (url_guild_id == cmd_data.cmd_guild->id && cmd_data.cmd_guild) {
        url_guild = cmd_data.cmd_guild;
    }
    else {
        const std::optional<std::shared_ptr<const dpp::guild>> url_guild_opt = co_await mln::caches::get_guild_full(url_guild_id, reply_log_data);
        if (!url_guild_opt.has_value()) {
            co_return;
        }
        url_guild = url_guild_opt.value();
    }

    //Retrieve url channel data
    std::shared_ptr<const dpp::channel> url_channel;
    if (url_channel_id == cmd_data.cmd_channel->id && cmd_data.cmd_channel) {
        url_channel = cmd_data.cmd_channel;
    }
    else {
        const std::optional<std::shared_ptr<const dpp::channel>> url_channel_opt = co_await mln::caches::get_channel_full(url_channel_id, reply_log_data);
        if (!url_channel_opt.has_value()) {
            co_return;
        }
        url_channel = url_channel_opt.value();
    }

    //Return error if the bot doesn't have the required permissions
    const std::optional<dpp::permission> url_bot_perm = co_await mln::perms::get_computed_permission_full(*url_guild, *url_channel, *cmd_data.cmd_bot, reply_log_data);
    if (!url_bot_perm.has_value()) {
        co_return;
    }
    if (!mln::perms::check_permissions(url_bot_perm.value(), dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed command, the bot doesn't have the permissions to retrieve the stored message!"), bot());
        co_return;
    }

    std::optional<dpp::message> stored_msg = co_await mln::caches::get_message_full(url_message_id, url_channel_id, reply_log_data);
    if (!stored_msg.has_value()) {
        co_return;
    }

    //Check other perms to send msg to user channel
    const dpp::permission to_check = co_await mln::perms::get_additional_perms_required(stored_msg.value(), bot(), cmd_data.cmd_guild->id);

    if (to_check != 0) {
        if (!mln::perms::check_permissions(cmd_data.cmd_bot_perm, to_check)) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed command, the bot doesn't have the additional permissions to send a msg to user channel!"), bot());
            co_return;
        }
    }

    //Verify if the attachments are already stored as file_data and, if not, iterate through all available attachments, download them and attach them to message to store in dump channel.
    if (stored_msg.value().attachments.size() != stored_msg.value().file_data.size()) {
        std::vector<std::tuple<dpp::http_request_completion_t, std::string, std::string>> download_results{};
        download_results.reserve(stored_msg.value().attachments.size());

        for (const dpp::attachment& attach : stored_msg.value().attachments) {
            //The attachment is not valid, return an error
            if (attach.owner == nullptr || attach.owner->owner == nullptr ||
                attach.id == 0 || attach.url.empty()) {

                mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                    "Failed to find valid attachment while selecting from database!"), bot(), &event_data,
                    std::format("Failed to find valid attachment while selecting from database! Owner: [{}], owner->owner: [{}], id: [{}], url: [{}].",
                        (attach.owner ? "not null" : "null"), (attach.owner ? (attach.owner->owner ? "not null" : "null") : "null"), static_cast<uint64_t>(attach.id), attach.url));
                co_return;
            }

            //Download attachments and add them to list of downloaded attachments, return an error if operation fails
            const dpp::http_request_completion_t download_res = co_await bot().co_request(attach.url, dpp::http_method::m_get, "", attach.content_type, {}, "1.1", mln::constants::get_big_files_request_timeout());
            if (download_res.status != 200) {
                mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                    "Failed to download valid attachment while selecting from database!"), bot(), &event_data,
                    std::format("Failed to download valid attachment while selecting from database! Status: [{}], error: [{}]", 
                        mln::get_http_err_text(download_res.status), mln::get_dpp_http_err_text(download_res.error)));
                co_return;
            }

            download_results.emplace_back(std::make_tuple(std::move(download_res), attach.filename, attach.content_type));
        }

        //Attach data to message
        stored_msg.value().attachments.clear();
        stored_msg.value().file_data.clear();
        for (const std::tuple<dpp::http_request_completion_t, std::string, std::string>& http_res_tuple : download_results) {
            stored_msg.value().add_file(std::get<1>(http_res_tuple), std::get<0>(http_res_tuple).body, std::get<2>(http_res_tuple));
        }
    }
    stored_msg.value().set_channel_id(cmd_data.cmd_channel->id).set_guild_id(cmd_data.cmd_guild->id);

    //Send the message to the target channel, depending on how 'broadcast' was set
    const dpp::command_value broadcast_param = event_data.get_parameter("broadcast");
    const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;
    
    if (broadcast) {
        const dpp::confirmation_callback_t send_result = co_await bot().co_message_create(stored_msg.value());
        if (send_result.is_error()) {
            const dpp::error_info err = send_result.get_error();

            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed command, the bot failed to send the message to the user's channel!"), bot(), &event_data,
                std::format("Failed command, the bot failed to send the message to the user's channel! Error: [{}], details: [{}]",
                    mln::get_json_err_text(err.code), err.human_readable));
            co_return;
        }

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Message retrieved!"), bot());

    } else {
        stored_msg.value().set_flags(dpp::m_ephemeral);

        const dpp::confirmation_callback_t edit_result = co_await event_data.co_edit_response(stored_msg.value());
        if (edit_result.is_error()) {
            const dpp::error_info err = edit_result.get_error();

            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                std::format("Failed command, the bot failed to send the message to the user's channel! The message size might be too large for ephemeral, try broadcast = true! Url: [{}].", std::get<0>(retrieved_data))), bot(), &event_data,
                std::format("Failed command, the bot failed to send the message to the user's channel! The message size might be too large for ephemeral, try broadcast = true! Error: [{}], details: [{}], url: [{}].", 
                    mln::get_json_err_text(err.code), err.human_readable, std::get<0>(retrieved_data)));
            co_return;
        }
    }
}

dpp::task<void> mln::db_select::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db select` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db select` set of commands is used to retrieve data from the database. By supplying a name, the bot will attempt to find a record with the same name and display the content. If the supplied name is not present in the database, an error will occur.

This is the main set of commands used to retrieve and display records inserted into the database.

**Types of select:**

- **/db select single**  
  *Parameters:* name[text, required], broadcast[boolean, optional].  
  This command asks for a record name and, optionally, a broadcast option. If broadcast is set to `false`, the result will only be shown to the user who invoked the command; if set to `true`, the result will be shown to everyone in the channel where the command was invoked.  
  If you have trouble remembering a record name, use the `/db show` commands to view all or some of the record names in the database, along with their descriptions (if present).)"""));

    if (mln::utility::conf_callback_is_error(co_await event_data.co_reply(s_info), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the db select help text!");
    }
    co_return;
}
