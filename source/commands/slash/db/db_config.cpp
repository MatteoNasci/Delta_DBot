#include "commands/slash/db/db_config.h"
#include "database/database_handler.h"
#include "utility/utility.h"
#include "utility/caches.h"
#include "utility/perms.h"
#include "utility/json_err.h"
#include "utility/response.h"

#include <dpp/cluster.h>

const std::unordered_map<mln::db_command_type, std::tuple<
    mln::db_init_type_flag,
    std::function<dpp::task<void>(const mln::db_config&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>>
    mln::db_config::s_mapped_commands_info{

    {mln::db_command_type::update_dump_channel, {db_init_type_flag::cmd_data | db_init_type_flag::thinking, &mln::db_config::update_dump}},
    {mln::db_command_type::help, {db_init_type_flag::none, &mln::db_config::help}},
};

mln::db_config::db_config(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, data{ .valid_stmt = true }, db{ in_db } {

    const mln::db_result_t res1 = db.save_statement("UPDATE OR ABORT guild_profile SET dedicated_channel_id = :CCC WHERE guild_id = :GGG RETURNING dedicated_channel_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save update_dump_channel stmt! Error: [{}], details: [{}].", 
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        data.valid_stmt = false;
    } else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":CCC", data.saved_param_channel);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save update_dump_channel stmt param indexes! guild_param: [{}, {}], channel_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
            data.valid_stmt = false;
        }
    }
}

mln::db_init_type_flag mln::db_config::get_requested_initialization_type(const db_command_type cmd) const {

    const auto it = s_mapped_commands_info.find(cmd);
    if (it == s_mapped_commands_info.end()) {
        return mln::db_init_type_flag::all;
    }
    return std::get<0>(it->second);
}

dpp::task<void> mln::db_config::update_dump(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    dpp::snowflake channel_id{0};
    const dpp::command_value channel_param = event_data.get_parameter("channel");
    if (std::holds_alternative<dpp::snowflake>(channel_param)) {
        channel_id = std::get<dpp::snowflake>(channel_param);
    }

    const std::optional<uint64_t> opt_channel = mln::caches::get_dump_channel_id(cmd_data.cmd_guild->id);//mln::caches::dump_channels_cache.get_element(cmd_data.cmd_guild->id);

    if (opt_channel.has_value()) {
        if (opt_channel.value() == channel_id) {
            mln::utility::conf_callback_is_error(
                co_await mln::response::make_response(false, event_data, "Channel found in cache. The given channel_id is already set as the dump channel!"), bot());
            co_return;
        }
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_channel, static_cast<int64_t>(channel_id));

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, "Failed to bind query params, internal error!"), bot(), &event_data, 
            std::format("Failed to bind query params, internal error! guild_param: [{}, {}], channel_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res1.type), res1.err_text, 
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    bool db_success = false;
    const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    const mln::db_result_t res = db.exec(data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !db_success) {
        const std::string err_text = ((!mln::database_handler::is_exec_error(res.type) && !db_success) ? 
            "Failed to update the dump channel, either no record found in the main database with the given guild id or you are not allowed to modify it!" : 
            "Failed to update the dump channel, internal error!");

        mln::utility::conf_callback_is_error(
            co_await mln::response::make_response(false, event_data, err_text), bot(), &event_data,
            std::format("{} Error: [{}], details: [{}].",
                err_text,
                mln::database_handler::get_name_from_result(res.type), res.err_text));

        mln::caches::dump_channels_cache.remove_element(cmd_data.cmd_guild->id);
        co_return;
    }

    mln::caches::dump_channels_cache.add_element(cmd_data.cmd_guild->id, channel_id);

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed update_dump_channel command conclusion reply!");
    }
}

dpp::task<void> mln::db_config::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db config` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db config` set of commands is used to configure the database environment, potentially altering the behavior of all other commands.

This set of commands is generally reserved for users with specific permissions.

**Types of config:**

- **/db config update_dump_channel**  
  *Parameters:* channel[text_channel, optional].  
  This command asks for a valid text channel to be used for storage purposes. If not provided, the current dump channel will be unset, and the database will use the channel where the database commands are used as the dump channel.  
  To avoid clutter, it is recommended to set a specific dump channel. The other database commands will use this channel to store data and records.  
  Ideally, this channel should be reserved for the bot’s use, and the messages created by the bot should not be edited or deleted, as this may cause the database to have broken records linking to content that has been modified or removed.)"""));

    if (mln::utility::conf_callback_is_error(co_await event_data.co_reply(s_info), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the db setup help text!");
    }
    co_return;
}

dpp::task<void> mln::db_config::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const {
    //Find the command variant and execute it. If no valid command variant found return an error
    const bool is_first_reply = (mln::db_config::get_requested_initialization_type(type) & mln::db_init_type_flag::thinking) == mln::db_init_type_flag::none;
    const auto it_func = s_mapped_commands_info.find(type);
    if (it_func == s_mapped_commands_info.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed command, the given sub_command is not supported!"), bot(), &event_data,
            std::format("Failed command, the given sub_command [{}] is not supported for /db setup!", mln::get_cmd_type_text(type)));
    }

    if (cmd_data.cmd_usr && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed database operation, admin permission is required to access this group command!"), bot());
        co_return;
    }
    
    if (!data.valid_stmt) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed database operation, the database was not initialized correctly!"), bot(), &event_data,
            "Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    co_await std::get<1>(it_func->second)(*this, event_data, cmd_data);
}
