#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "commands/slash/db/db_update.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_result.h"
#include "database/db_saved_stmt_state.h"
#include "database/db_text_encoding.h"
#include "enum/flags.h"
#include "utility/caches.h"
#include "utility/constants.h"
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

#include <cstdint>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>
#include <format>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

mln::db_update::db_update(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, 
data{ .state = db_saved_stmt_state::none }, data_nsfw{ .state = db_saved_stmt_state::none }, db{ in_db } {

    const mln::db_result_t res1 = db.save_statement("UPDATE OR ABORT storage SET desc = :DDD WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update description stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
    }
    else {
        data.state = mln::flags::add(data.state, db_saved_stmt_state::stmt_initialized);
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_to_update);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok) {
            cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update description stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}], desc_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text));

        }
        else {
            data.state = mln::flags::add(data.state, db_saved_stmt_state::params_initialized);
        }
    }

    const mln::db_result_t res2 = db.save_statement("UPDATE OR ABORT storage SET nsfw = :WWW WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", data_nsfw.saved_stmt);
    if (res2.type != mln::db_result::ok) {
        cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update nsfw stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res2.type), res2.err_text));
    }
    else {
        data_nsfw.state = mln::flags::add(data_nsfw.state, db_saved_stmt_state::stmt_initialized);
        const mln::db_result_t res11 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":GGG", data_nsfw.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":NNN", data_nsfw.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":UUU", data_nsfw.saved_param_user);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":WWW", data_nsfw.saved_param_to_update);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok) {
            cbot().log(dpp::loglevel::ll_error, std::format("Failed to save update nsfw stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}], desc_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text));
        }
        else {
            data_nsfw.state = mln::flags::add(data_nsfw.state, db_saved_stmt_state::params_initialized);
        }
    }

    cbot().log(dpp::loglevel::ll_debug, std::format("db_update: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

mln::db_update::~db_update()
{
    if (mln::flags::has(data.state, db_saved_stmt_state::stmt_initialized)) {
        db.delete_statement(data.saved_stmt);
    }
    if (mln::flags::has(data_nsfw.state, db_saved_stmt_state::stmt_initialized)) {
        db.delete_statement(data_nsfw.saved_stmt);
    }
}

mln::db_update::db_update(db_update&& rhs) noexcept : base_db_command{ std::forward<db_update>(rhs) }, data{ rhs.data }, db{ rhs.db }
{
    rhs.data.state = db_saved_stmt_state::none;
    rhs.data_nsfw.state = db_saved_stmt_state::none;
}

mln::db_update& mln::db_update::operator=(db_update&& rhs) noexcept
{
    base_db_command::operator=(std::forward<db_update>(rhs));

    data = rhs.data;
    rhs.data.state = db_saved_stmt_state::none;
    rhs.data_nsfw.state = db_saved_stmt_state::none;

    return *this;
}

dpp::task<void> mln::db_update::command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) {

    switch (type) {
    case mln::db_command_type::description:
        co_await mln::db_update::description(event_data, cmd_data);
        break;
    case mln::db_command_type::nsfw:
        co_await mln::db_update::nsfw(event_data, cmd_data);
        break;
    case mln::db_command_type::help:
        co_await mln::db_update::help(cmd_data);
        break;
    default:
        co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
            std::format("Failed command, the given sub_command [{}] is not supported for /db update!", mln::get_cmd_type_text(type)));
        break;
    }
}

mln::db_init_type_flag mln::db_update::get_requested_initialization_type(const db_command_type cmd) const noexcept {
    switch (cmd) {
    case mln::db_command_type::description:
    case mln::db_command_type::nsfw:
        return mln::flags::add(db_init_type_flag::cmd_data, db_init_type_flag::thinking);
    case mln::db_command_type::help:
        return db_init_type_flag::none;
    default:
        return mln::db_init_type_flag::all;
    }
}

mln::db_saved_stmt_state mln::db_update::is_db_initialized() const noexcept
{
    return mln::flags::com(data.state, data_nsfw.state);
}

dpp::task<void> mln::db_update::description(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const {
    //Retrieve remaining data required for the database query
    std::string description;
    const dpp::command_value& desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        description = std::get<std::string>(desc_param);

        if (!(co_await mln::utility::check_text_validity(description, cmd_data.data, false,
            mln::constants::get_min_characters_description(), mln::constants::get_max_characters_description(), "description"))) {
            co_return;
        }

        if (!mln::utility::is_ascii_printable(description)) {
            co_await mln::response::co_respond(cmd_data.data,
                "Failed to bind query parameters, given description is composed of invalid characters! Only ASCII printable characters are accepted [32,126]", false, {});
            co_return;
        }
    }

    mln::db_result_t res;
    if (valid_description) {
        res = db.bind_parameter(data.saved_stmt, 0, data.saved_param_to_update, description, mln::db_text_encoding::utf8);
    }
    else {
        res = db.bind_parameter(data.saved_stmt, 0, data.saved_param_to_update);
    }

    //Check if any error occurred in the binding process, in case return an error
    if (res.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
            std::format("Failed to bind query parameters, internal error! desc_param: [{}, {}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await common(event_data, cmd_data, data);
}

dpp::task<void> mln::db_update::nsfw(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const
{
    //Retrieve remaining data required for the database query
    const dpp::command_value& nsfw_param = event_data.get_parameter("nsfw");
    if (!std::holds_alternative<bool>(nsfw_param)) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to retrieve nsfw tag parameter!", true, "Failed to retrieve nsfw tag parameter!");
        co_return;
    }

    const bool nsfw = std::get<bool>(nsfw_param);

    const mln::db_result_t res = db.bind_parameter(data_nsfw.saved_stmt, 0, data.saved_param_to_update, static_cast<int>(nsfw));

    //Check if any error occurred in the binding process, in case return an error
    if (res.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
            std::format("Failed to bind query parameters, internal error! nsfw_param: [{}, {}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await common(event_data, cmd_data, data_nsfw);
}

dpp::task<void> mln::db_update::common(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const mln::db_update::data_t& stmt_data) const {
    //Retrieve remaining data required for the database query
    const dpp::command_value& name_param = event_data.get_parameter("name");

    const std::optional<std::string> name = co_await mln::utility::check_text_validity(name_param, cmd_data.data, false,
        mln::constants::get_min_characters_text_id(), mln::constants::get_max_characters_text_id(), "record name");

    if (!name.has_value()) {
        co_return;
    }

    if (!mln::utility::is_ascii_printable(name.value())) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]", true,
            "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]");
        co_return;
    }

    const dpp::command_value& owner_param = event_data.get_parameter("owner");
    const uint64_t target = std::holds_alternative<dpp::snowflake>(owner_param) ? static_cast<uint64_t>(std::get<dpp::snowflake>(owner_param)) : cmd_data.data.usr_id;
    if (target != cmd_data.data.usr_id) {
        if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
            co_await mln::response::co_respond(cmd_data.data, "Failed to update the given record, admin permission required to update records owned by someone else!", false, {});
            co_return;
        }
    }

    //Bind query parameters
    const mln::db_result_t res1 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_user, static_cast<int64_t>(target));
    const mln::db_result_t res3 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_name, name.value(), mln::db_text_encoding::utf8);

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
            std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], user_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        co_return;
    }

    //Prepare callbacks for query execution
    bool db_success = false;
    const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    //Execute query and return an error if the query failed or if no element was found
    const mln::db_result_t res = db.exec(stmt_data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !db_success) {
        const std::string err_text = ((!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !db_success) ?
            "Failed while executing database query! The given name was not found in the database or you are not the owner of the record!" :
            "Failed while executing database query! Internal error!";

        co_await mln::response::co_respond(cmd_data.data, err_text, true, std::format("{} Error: [{}], details: [{}].",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({ cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id });

    co_await mln::response::co_respond(cmd_data.data, "Database operation successful!", false, "Failed update command conclusion reply!");
}

dpp::task<void> mln::db_update::help(db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{ "Information regarding the `/db update` commands..." }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db update` commands are designed to update information contained in records stored in the database related to the current Discord server.

The bot will assume that the command user owns the record being updated. If this is not the case, an error will occur.

Only ASCII printable characters are accepted as input for the `name` and `description` parameters.

The optional owner parameter can be used by admins to update records owned by other people.

**Types of update:**

- **/db update description**  
  *Parameters:* name[text, required], owner[user ID, optional], description[text, optional].  
  This command searches for a record in the database identified by the given name and owned by the command user. If the record exists, its description will be updated to the provided description text. If no description is provided, the record's description will be left blank.
  The owner parameter allows the user to update a record owned by the given owner. Only admins can use this feature.

- **/db update nsfw**  
  *Parameters:* name[text, required], owner[user ID, optional], nsfw[boolean, required].  
  This command searches for a record in the database identified by the given name and owned by the command user. If the record exists, its nsfw tag will be updated to the provided nsfw boolean value.
  The owner parameter allows the user to update a record owned by the given owner. Only admins can use this feature.)"""));

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the db update help text!");
    co_return;
}