#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "commands/slash/db/db_show.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_column_data.h"
#include "database/db_result.h"
#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"
#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>

#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

static constexpr uint64_t s_paginated_embed_time_limit{300};
static constexpr uint64_t s_paginated_embed_text_limit{2000};

mln::db_show::db_show(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, 
data{ .valid_all = db_saved_stmt_state::none, .valid_user = db_saved_stmt_state::none }, db{ in_db } {
    const mln::db_result_t res1 = db.save_statement("SELECT name, nsfw, desc, user_id FROM storage WHERE guild_id = ?1 ORDER BY user_id ASC, creation_time ASC;", data.saved_stmt_all);
    if (res1.type != mln::db_result::ok) {
        cbot().log(dpp::loglevel::ll_error, std::format("Failed to save show all stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
    }
    else {
        data.valid_all = mln::flags::add(data.valid_all, db_saved_stmt_state::initialized);
    }

    const mln::db_result_t res2 = db.save_statement("SELECT name, nsfw, desc FROM storage WHERE guild_id = :GGG AND user_id = :UUU ORDER BY creation_time ASC;", data.saved_stmt_user);
    if (res2.type != mln::db_result::ok) {
        cbot().log(dpp::loglevel::ll_error, std::format("Failed to save show user stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res2.type), res2.err_text));
    } else {
        data.valid_user = mln::flags::add(data.valid_user, db_saved_stmt_state::stmt_initialized);
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt_user, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt_user, 0, ":UUU", data.saved_param_user);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            cbot().log(dpp::loglevel::ll_error, std::format("Failed to save select stmt param indexes! guild_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
        }
        else {
            data.valid_user = mln::flags::add(data.valid_user, db_saved_stmt_state::params_initialized);
        }
    }

    cbot().log(dpp::loglevel::ll_debug, std::format("db_show: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

mln::db_show::~db_show()
{
    if (mln::flags::has(data.valid_all, db_saved_stmt_state::stmt_initialized)) {
        db.delete_statement(data.saved_stmt_all);
    }
    if (mln::flags::has(data.valid_user, db_saved_stmt_state::stmt_initialized)) {
        db.delete_statement(data.saved_stmt_user);
    }
}

mln::db_show::db_show(db_show&& rhs) noexcept : base_db_command{ std::forward<db_show>(rhs) }, data{ rhs.data }, db{ rhs.db }
{
    rhs.data.valid_all = db_saved_stmt_state::none;
    rhs.data.valid_user = db_saved_stmt_state::none;
}

mln::db_show& mln::db_show::operator=(db_show&& rhs) noexcept
{
    if (this != &rhs) {
        base_db_command::operator=(std::forward<db_show>(rhs));

        data = rhs.data;
        rhs.data.valid_all = db_saved_stmt_state::none;
        rhs.data.valid_user = db_saved_stmt_state::none;
    }

    return *this;
}

dpp::task<void> mln::db_show::command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) {

    switch (type) {
    case mln::db_command_type::all:
        co_await mln::db_show::all(event_data, cmd_data);
        break;
    case mln::db_command_type::user:
        co_await mln::db_show::user(event_data, cmd_data);
        break;
    case mln::db_command_type::help:
        co_await mln::db_show::help(cmd_data);
        break;
    default:
        co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
            std::format("Failed command, the given sub_command [{}] is not supported for /db show!", mln::get_cmd_type_text(type)));
        break;
    }
}

mln::db_init_type_flag mln::db_show::get_requested_initialization_type(const db_command_type cmd) const noexcept  {
    switch (cmd) {
    case mln::db_command_type::all:
    case mln::db_command_type::user:
        return mln::flags::add(db_init_type_flag::cmd_data, db_init_type_flag::thinking);
    case mln::db_command_type::help:
        return db_init_type_flag::none;
    default:
        return db_init_type_flag::all;
    }
}
mln::db_saved_stmt_state mln::db_show::is_db_initialized() const noexcept
{
    return mln::flags::com(data.valid_all, data.valid_user);
}
dpp::task<void> mln::db_show::execute_show(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const exec_show_t& stmt_data, const std::optional<std::shared_ptr<const std::vector<std::string>>>& cached_show) const {
    const bool use_cache = cached_show.has_value() && cached_show.value();

    //If we found the text data from cache there's no need to execute the SQL query. If not found, then we contact the database (and store the result in cache as well)
    std::shared_ptr<const std::vector<std::string>> records;

    if (use_cache) {
        records = cached_show.value();
    }else{
        //Prepare callbacks for query execution
        std::vector<std::string> temp_records{};
        std::vector<uint64_t> temp_users{};
        std::string temp_name{};
        bool temp_nsfw{};
        mln::database_callbacks_t calls{};
        const bool all_stmt = stmt_data.stmt == data.saved_stmt_all;
        if (all_stmt) {
            calls.type_definer_callback = [](void*, int c) constexpr {return c != 3;};
            calls.data_adder_callback = [&temp_records, &temp_name, &temp_users, &temp_nsfw](void*, int c, mln::db_column_data_t&& d) constexpr {
                switch (c) {
                case 0:
                    temp_name = std::string(reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data))));
                    break;
                case 1:
                    temp_nsfw = static_cast<bool>(std::get<int>(d.data));
                    break;
                case 2:
                    if (std::holds_alternative<const short*>(d.data)) {
                        temp_records.emplace_back("name: [" + temp_name + "], nsfw: [" + (temp_nsfw ? "true" : "false") + "] }");
                    }
                    else {
                        temp_records.emplace_back("name: [" + 
                            temp_name + "], description: [" + 
                            std::string{ reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)) } + "], nsfw: [" + 
                            (temp_nsfw ? "true" : "false") + "] }");
                    }
                    break;
                case 3:
                    temp_users.emplace_back(static_cast<uint64_t>(std::get<int64_t>(d.data)));
                    break;
                }                
            };
        } else {
            calls.type_definer_callback = [](void*, int) constexpr {return true;};
            calls.data_adder_callback = [&temp_records, &temp_name, &temp_nsfw](void*, int c, mln::db_column_data_t&& d) constexpr {
                switch (c) {
                case 0:
                    temp_name = std::string(reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data))));
                    break;
                case 1:
                    temp_nsfw = static_cast<bool>(std::get<int>(d.data));
                    break;
                case 2:
                    if (std::holds_alternative<const short*>(d.data)) {
                        temp_records.emplace_back("{ name: [" + temp_name + "], nsfw: [" + (temp_nsfw ? "true" : "false") + "] }");
                    }
                    else {
                        temp_records.emplace_back("{ name: [" + 
                            temp_name + "], description: [" + 
                            std::string{ reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)) } + "], nsfw: [" + 
                            (temp_nsfw ? "true" : "false") + "] }");
                    }
                    break;
                }
            };
        }

        //Execute query and return an error if the query failed or if no element was added
        const mln::db_result_t res = db.exec(stmt_data.stmt, calls);
        if (mln::database_handler::is_exec_error(res.type) || temp_records.empty()) {
            const std::string err_text = !mln::database_handler::is_exec_error(res.type) && temp_records.empty() ?
                "Failed while executing database query!" " No record found in the database!" :
                "Failed while executing database query! Internal error!";

            co_await mln::response::co_respond(cmd_data.data, err_text, true,
                std::format("{} Error: [{}], details: [{}].", err_text, mln::database_handler::get_name_from_result(res.type), res.err_text));
            co_return;
        }

        //Update cache
        if (all_stmt) {

            if (temp_records.size() != temp_users.size()) {
                co_await mln::response::co_respond(cmd_data.data, "Failed while executing database query! Internal error while extracting db data!", true,
                    std::format("Failed while executing database query! Internal error while extracting db data! Records size: [{}], users size: [{}].", temp_records.size(), temp_users.size()));
                co_return;
            }

            for (size_t i = 0; i < temp_records.size(); ++i) {
                const uint64_t user_id = temp_users[i];
                const std::optional<std::shared_ptr<const dpp::user_identified>> user = co_await mln::caches::get_user_task(user_id, cmd_data.data, &event_data.command.usr, &event_data.command.resolved.users);

                if (!user.has_value()) {
                    co_return;
                }

                temp_records[i] = "{ owner: [" + (user.value()->global_name.empty() ? std::to_string(user.value()->id) : user.value()->global_name) + "], " + temp_records[i];
            }

            records = mln::caches::show_all_cache.add_element(cmd_data.data.guild_id, std::move(temp_records));
        } else {
            records = mln::caches::show_user_cache.add_element({cmd_data.data.guild_id, stmt_data.target}, std::move(temp_records));
        }
    }

    //Create paginated embed job
    mln::utility::manage_paginated_embed(mln::utility::paginated_data_t{
        .event_data = cmd_data.data,
        .time_limit_seconds = s_paginated_embed_time_limit, 
        .text_limit = s_paginated_embed_text_limit }, records);
}

dpp::task<void> mln::db_show::all(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const {
    //Bind query parameters
    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt_all, 0, 1, static_cast<int64_t>(cmd_data.data.guild_id));

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
            std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}].", mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        co_return;
    }

    co_await execute_show(event_data, cmd_data, exec_show_t{.stmt = data.saved_stmt_all, .target = 0}, mln::caches::get_show_all(cmd_data.data.guild_id));
}

dpp::task<void> mln::db_show::user(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data) const {
    //Bind query parameters
    const dpp::command_value& user_param = event_data.get_parameter("user");
    const dpp::snowflake target = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ 0 };
    if (target == 0) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to retrieve user parameter!", true, "Failed to retrieve user parameter!");
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt_user, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt_user, 0, data.saved_param_user, static_cast<int64_t>(target));

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
            std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], user_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res1.type), res1.err_text, 
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    co_await execute_show(event_data, cmd_data, exec_show_t{.stmt = data.saved_stmt_user, .target = target}, mln::caches::get_show_user(cmd_data.data.guild_id, target));
}

dpp::task<void> mln::db_show::help(db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db show` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db show` commands are designed to display various records stored in the database related to the current Discord server.

These commands are used to verify records within a Discord server, but they do not display the content of the stored messages.

You can use `/db show` to locate a specific record and then use its name (the `record ID`) to retrieve the associated message with the `/db select` commands.

**Types of show:**

- **/db show all**  
  *Parameters:* none.  
  Displays a paginated list of all current (at the time this command was invoked) records in the database, formatted as:

  `{ owner: [record owner], name: [record ID], description: [record description], nsfw: [True/False] }`.

  The description field is omitted if not available.

- **/db show user**  
  *Parameters:* user[user ID, required].  
  Displays a paginated list of all current (at the time this command was invoked) records in the database related to the specified user, formatted as:

  `{ name: [record ID], description: [record description], nsfw: [True/False] }`.

  The description field is omitted if not available.)"""));

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the db show help text!");
    co_return;
}
