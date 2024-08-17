#include "commands/slash/db/db_delete.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"

mln::db_delete::db_delete(bot_delta* const delta) : base_db_command(delta),
saved_stmt(), saved_param_guild(), saved_param_user(), saved_param_name(), valid_stmt(true) {

    mln::db_result res1 = delta->db.save_statement("DELETE FROM storage WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":GGG", saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":NNN", saved_param_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":UUU", saved_param_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt param indexes!");
            valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_delete::command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) {
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    if (!valid_stmt) {
        co_await thinking;
        event_data.edit_response("Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    const dpp::snowflake guild_id = event_data.command.guild_id;
    const dpp::snowflake user_id = event_data.command.usr.id;

    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    mln::db_result res1 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_guild, static_cast<int64_t>(guild_id));
    mln::db_result res2 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_user, static_cast<int64_t>(user_id));
    mln::db_result res3 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_name, name.c_str(), name.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);

    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
        co_await thinking;
        event_data.edit_response("Failed to bind query parameters, internal error!");
        co_return;
    }

    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback();
    calls.callback_data = &db_success;

    dpp::message msg{};
    mln::db_result res = delta()->db.exec(saved_stmt, calls);
    if (res != mln::db_result::ok || !db_success) {
        msg.set_content(res == mln::db_result::ok && !db_success ? "Failed to delete element, either no record found with the given name or you are not allowed to modify the record!" : "Failed to delete element, internal error!");
    } else {
        msg.set_content("Element deleted from the db!");
    }

    co_await thinking;
    event_data.edit_response(msg);
}
