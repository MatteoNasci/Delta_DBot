#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_config.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_result.h"
#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"
#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

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

mln::db_config::db_config(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, 
data { .state = db_saved_stmt_state::none }, 
db{ in_db } {
    const mln::db_result_t res1 = db.save_statement("UPDATE OR ABORT guild_profile SET dedicated_channel_id = :CCC WHERE guild_id = :GGG RETURNING dedicated_channel_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update_dump_channel stmt! Error: [{}], details: [{}].", 
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
    } else {
        data.state = mln::flags::add(data.state, db_saved_stmt_state::stmt_initialized);

        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":CCC", data.saved_param_channel);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update_dump_channel stmt param indexes! guild_param: [{}, {}], channel_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
        }
        else {
            data.state = mln::flags::add(data.state, db_saved_stmt_state::params_initialized);
        }
    }

    cbot().log(dpp::loglevel::ll_debug, std::format("db_config: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

mln::db_config::~db_config()
{
    if (mln::flags::has(is_db_initialized(), db_saved_stmt_state::stmt_initialized)) {
        db.delete_statement(data.saved_stmt);
    }
}

mln::db_config::db_config(db_config&& rhs) noexcept : base_db_command{ std::forward<db_config>(rhs) }, data{ rhs.data }, db{ rhs.db }
{
    rhs.data.state = db_saved_stmt_state::none;
}

mln::db_config& mln::db_config::operator=(db_config&& rhs) noexcept
{
    base_db_command::operator=(std::forward<db_config>(rhs));
    data = rhs.data;

    rhs.data.state = db_saved_stmt_state::none;

    return *this;
}

mln::db_init_type_flag mln::db_config::get_requested_initialization_type(const db_command_type cmd) const noexcept {
    switch (cmd) {
    case mln::db_command_type::update_dump_channel:
        return mln::flags::add(db_init_type_flag::cmd_data, db_init_type_flag::thinking);
    case mln::db_command_type::help:
        return db_init_type_flag::none;
    default:
        return mln::db_init_type_flag::all;
    }
}

mln::db_saved_stmt_state mln::db_config::is_db_initialized() const noexcept
{
    return data.state;
}

dpp::task<void> mln::db_config::update_dump(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const {
    dpp::snowflake channel_id{0};
    const dpp::command_value& channel_param = event_data.get_parameter("channel");
    if (std::holds_alternative<dpp::snowflake>(channel_param)) {
        channel_id = std::get<dpp::snowflake>(channel_param);
    }

    const std::optional<uint64_t> opt_channel = mln::caches::get_dump_channel_id(cmd_data.data.guild_id);

    if (opt_channel.has_value()) {
        if (opt_channel.value() == channel_id) {
            co_await mln::response::co_respond(cmd_data.data, "Channel found in cache. The given channel_id is already set as the dump channel!", false, {});
            co_return;
        }
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_channel, static_cast<int64_t>(channel_id));

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query params, internal error!", true,
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

        co_await mln::response::co_respond(cmd_data.data, err_text, true,
            std::format("{} Error: [{}], details: [{}].",
                err_text,
                mln::database_handler::get_name_from_result(res.type), res.err_text));

        mln::caches::dump_channels_cache.remove_element(cmd_data.data.guild_id);
        co_return;
    }

    mln::caches::dump_channels_cache.add_element(cmd_data.data.guild_id, channel_id);

    co_await mln::response::co_respond(cmd_data.data, "Database operation successful!", false, "Failed update_dump_channel command conclusion reply!");
}

dpp::task<void> mln::db_config::help(db_cmd_data_t& cmd_data) const {

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the db setup help text!");
    co_return;
}

dpp::task<void> mln::db_config::command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) {

    if (cmd_data.cmd_usr && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(cmd_data.data, "Failed database operation, admin permission is required to access this group command!", false, {});
        co_return;
    }
    
    switch (type) {
    case mln::db_command_type::update_dump_channel:
        co_await mln::db_config::update_dump(event_data, cmd_data);
        break;
    case mln::db_command_type::help:
        co_await mln::db_config::help(cmd_data);
        break;
    default:
        co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
            std::format("Failed command, the given sub_command [{}] is not supported for /db setup!", mln::get_cmd_type_text(type)));
        break;
    }
}
