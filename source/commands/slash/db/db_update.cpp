#include "commands/slash/db/db_update.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "database/database_handler.h"

mln::db_update::db_update(bot_delta* const delta) : base_db_command(delta), data{ .valid_stmt = true } {

    mln::db_result res1 = delta->db.save_statement("UPDATE OR ABORT storage SET desc = :DDD WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", data.saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update description stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    }
    else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        mln::db_result res14 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update description stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_update::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const std::unordered_map<mln::db_command_type, std::function<dpp::task<void>(mln::db_update*, const dpp::slashcommand_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>> s_allowed_subcommands{
        {db_command_type::description, &mln::db_update::description},
        {db_command_type::help, &mln::db_update::help},
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

mln::db_init_type_flag mln::db_update::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::description, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::help, db_init_type_flag::none},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}


dpp::task<void> mln::db_update::description(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking){
    //Retrieve remaining data required for the database query
    std::string description;
    const dpp::command_value desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        description = std::get<std::string>(desc_param);

        if (!mln::utility::is_ascii_printable(description)) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Failed to bind query parameters, given description is composed of invalid characters! Only ASCII printable characters are accepted [32,126]");
            co_return;
        }
    }
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));
    if (!mln::utility::is_ascii_printable(name)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]");
        co_return;
    }

    //Bind query parameters
    mln::db_result res1 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_user, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    mln::db_result res3 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name, mln::db_text_encoding::utf8);
    mln::db_result res4;
    if (valid_description) {
        res4 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc, description, mln::db_text_encoding::utf8);
    }
    else {
        res4 = delta()->db.bind_parameter(data.saved_stmt, 0, data.saved_param_desc);
    }

    //Check if any error occurred in the binding process, in case return an error
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok || res4 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to bind query parameters, internal error! " + delta()->db.get_last_err_msg() +
            ": " + mln::database_handler::get_name_from_result(res1) + ", " + mln::database_handler::get_name_from_result(res2) + ", " +
            mln::database_handler::get_name_from_result(res3) + ", " + mln::database_handler::get_name_from_result(res4));
        co_return;
    }

    //Prepare callbacks for query execution
    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    //Execute query and return an error if the query failed or if no element was found
    mln::db_result res = delta()->db.exec(data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res) || !db_success) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, (!mln::database_handler::is_exec_error(res) || res == mln::db_result::constraint_primary_key) && !db_success ?
            "Failed while executing database query! The given name was not found in the database or you are not the owner of the record!" :
            "Failed while executing database query! Internal error! " + mln::database_handler::get_name_from_result(res));
        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({ cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id });

    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
        "Description updated!", {false, dpp::loglevel::ll_debug});
}

dpp::task<void> mln::db_update::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{ "Information regarding the `/db update` commands..." }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db update` commands are designed to update information contained in records stored in the database related to the current Discord server.

The bot will assume that the command user owns the record being updated. If this is not the case, an error will occur.

Only ASCII printable characters are accepted as input for the `name` and `description` parameters.

**Types of update:**

- **/db update description**  
  *Parameters:* name[text, required], description[text, optional].  
  This command searches for a record in the database identified by the given name and owned by the command user. If the record exists, its description will be updated to the provided description text. If no description is provided, the record's description will be left blank.)"""));

    event_data.reply(dpp::message{ s_info });
    co_return;
}
