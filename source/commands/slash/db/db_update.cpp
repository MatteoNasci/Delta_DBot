#include "commands/slash/db/db_update.h"
#include "database/database_handler.h"
#include "bot_delta.h"

mln::db_update::db_update(bot_delta* const delta) : db_insert(delta, {.valid_stmt = true}, "Failed while executing database query, either no record found in the database with the given name, you are not allowed to modify it or you are trying to change a record from file type to text type (or vice versa)!!") {

    const mln::db_result res1 = delta->db.save_statement("UPDATE OR ABORT storage SET url = :LLL, desc = :DDD WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU AND url_type = :TTT RETURNING user_id;", data.saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":LLL", data.saved_param_url);
        mln::db_result res14 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":TTT", data.saved_param_url_type);
        mln::db_result res15 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":UUU", data.saved_param_user);
        mln::db_result res16 = delta->db.get_bind_parameter_index(data.saved_stmt, 0, ":DDD", data.saved_param_desc);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok || res14 != mln::db_result::ok || res15 != mln::db_result::ok || res16 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}
