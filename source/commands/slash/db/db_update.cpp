#include "commands/slash/db/db_update.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "database/database_handler.h"
#include "utility/response.h"

#include <dpp/cluster.h>

const std::unordered_map<mln::db_command_type, std::tuple<
    mln::db_init_type_flag,
    std::function<dpp::task<void>(const mln::db_update&, const dpp::slashcommand_t&, const mln::db_cmd_data_t&)>>>
    mln::db_update::s_mapped_commands_info{

    {mln::db_command_type::description, {db_init_type_flag::cmd_data | db_init_type_flag::thinking, &mln::db_update::description}},
    {mln::db_command_type::nsfw, {db_init_type_flag::cmd_data | db_init_type_flag::thinking, &mln::db_update::nsfw}},
    {mln::db_command_type::help, {db_init_type_flag::none, &mln::db_update::help}},
};

mln::db_update::db_update(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, data{ .valid_stmt = true }, db{ in_db } {

    const mln::db_result_t res1 = db.save_statement("UPDATE OR ABORT storage SET desc = :DDD WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save update description stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_to_update);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save update description stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}], desc_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text));

            data.valid_stmt = false;
        }
    }

    const mln::db_result_t res2 = db.save_statement("UPDATE OR ABORT storage SET nsfw = :WWW WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", data_nsfw.saved_stmt);
    if (res2.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save update nsfw stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":GGG", data_nsfw.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":NNN", data_nsfw.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":UUU", data_nsfw.saved_param_user);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data_nsfw.saved_stmt, 0, ":WWW", data_nsfw.saved_param_to_update);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save update nsfw stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}], desc_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text));
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_update::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const {

    //Find the command variant and execute it. If no valid command variant found return an error
    const bool is_first_reply = (mln::db_update::get_requested_initialization_type(type) & mln::db_init_type_flag::thinking) == mln::db_init_type_flag::none;
    const auto it_func = s_mapped_commands_info.find(type);
    if (it_func == s_mapped_commands_info.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed command, the given sub_command is not supported!"), bot(), &event_data,
            std::format("Failed command, the given sub_command [{}] is not supported for /db update!", mln::get_cmd_type_text(type)));
        co_return;
    }

    //If the query statement was not saved correctly, return an error
    if (!data.valid_stmt || !data_nsfw.valid_stmt) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data,
            "Failed database operation, the database was not initialized correctly!"), bot(), &event_data,
            "Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    co_await std::get<1>(it_func->second)(*this, event_data, cmd_data);
}

mln::db_init_type_flag mln::db_update::get_requested_initialization_type(const db_command_type cmd) const {

    const auto it = s_mapped_commands_info.find(cmd);
    if (it == s_mapped_commands_info.end()) {
        return mln::db_init_type_flag::all;
    }
    return std::get<0>(it->second);
}


dpp::task<void> mln::db_update::description(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    //Retrieve remaining data required for the database query
    std::string description;
    const dpp::command_value desc_param = event_data.get_parameter("description");
    const bool valid_description = std::holds_alternative<std::string>(desc_param);
    if (valid_description) {
        description = std::get<std::string>(desc_param);

        if (!mln::utility::is_ascii_printable(description)) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Failed to bind query parameters, given description is composed of invalid characters! Only ASCII printable characters are accepted [32,126]"), bot());
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
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, internal error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal error! desc_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await common(event_data, cmd_data, data);
}

dpp::task<void> mln::db_update::nsfw(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const
{
    //Retrieve remaining data required for the database query
    const bool nsfw = std::get<bool>(event_data.get_parameter("nsfw"));

    const mln::db_result_t res = db.bind_parameter(data_nsfw.saved_stmt, 0, data.saved_param_to_update, static_cast<int>(nsfw));

    //Check if any error occurred in the binding process, in case return an error
    if (res.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, internal error!"), bot(), &event_data,
            std::format("Failed to bind query parameters, internal error! nsfw_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await common(event_data, cmd_data, data_nsfw);
}

dpp::task<void> mln::db_update::common(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const mln::db_update::data_t& stmt_data) const {
    //Retrieve remaining data required for the database query
    const std::string name = std::get<std::string>(event_data.get_parameter("name"));
    if (!mln::utility::is_ascii_printable(name)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, given name is composed of invalid characters! Only ASCII printable characters are accepted [32,126]"), bot());
        co_return;
    }

    //Bind query parameters
    const mln::db_result_t res1 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_user, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    const mln::db_result_t res3 = db.bind_parameter(stmt_data.saved_stmt, 0, stmt_data.saved_param_name, name, mln::db_text_encoding::utf8);

    //Check if any error occurred in the binding process, in case return an error
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            "Failed to bind query parameters, internal error!"), bot(), &event_data,
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

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
            err_text), bot(), &event_data,
            std::format("{} Error: [{}], details: [{}].",
                err_text,
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    //Update caches
    mln::caches::show_all_cache.remove_element(cmd_data.cmd_guild->id);
    mln::caches::show_user_cache.remove_element({ cmd_data.cmd_guild->id, cmd_data.cmd_usr->user_id });

    if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Database operation successful!"), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed update command conclusion reply!");
    }
}

dpp::task<void> mln::db_update::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{ "Information regarding the `/db update` commands..." }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db update` commands are designed to update information contained in records stored in the database related to the current Discord server.

The bot will assume that the command user owns the record being updated. If this is not the case, an error will occur.

Only ASCII printable characters are accepted as input for the `name` and `description` parameters.

**Types of update:**

- **/db update description**  
  *Parameters:* name[text, required], description[text, optional].  
  This command searches for a record in the database identified by the given name and owned by the command user. If the record exists, its description will be updated to the provided description text. If no description is provided, the record's description will be left blank.

- **/db update nsfw**  
  *Parameters:* name[text, required], nsfw[boolean, required].  
  This command searches for a record in the database identified by the given name and owned by the command user. If the record exists, its nsfw tag will be updated to the provided nsfw boolean value.)"""));

    if (mln::utility::conf_callback_is_error(co_await event_data.co_reply(s_info), bot())) {
        mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the db update help text!");
    }
    co_return;
}