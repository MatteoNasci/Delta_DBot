#include "commands/guild/create/insert_guild_db.h"
#include "bot_delta.h"

mln::insert_guild_db::insert_guild_db(bot_delta* const in_delta) : base_guild_create(in_delta), saved_insert_guild_query(), valid_saved_insert(true) {
    auto res = delta()->db.save_statement("INSERT OR IGNORE INTO guild_profile(guild_id) VALUES(?);", saved_insert_guild_query);
    if (res != mln::db_result::ok) {
        delta()->bot.log(dpp::loglevel::ll_error, "Failed to save insert guild query statement! " + mln::database_handler::get_name_from_result(res));
        valid_saved_insert = false;
    }
}

dpp::job mln::insert_guild_db::command(std::shared_ptr<dpp::guild_create_t> event) {
    const int64_t guild_id = static_cast<int64_t>(event->created->id);

    if (!valid_saved_insert) {
        delta()->bot.log(dpp::loglevel::ll_error, "Failed to insert guild in the db! The save insert stmt was not saved correctly.");
        co_return;
    }

    auto res = delta()->db.bind_parameter(saved_insert_guild_query, 0, 1, guild_id);
    if (res != mln::db_result::ok) {
        delta()->bot.log(dpp::loglevel::ll_error, "Failed to bind guild " + std::to_string(guild_id) + " to insert guild stmt! " + mln::database_handler::get_name_from_result(res) + ". " + delta()->db.get_last_err_msg());
        co_return;
    }
    
    res = delta()->db.exec(saved_insert_guild_query, mln::database_callbacks_t());
    if (res != mln::db_result::ok) {
        delta()->bot.log(dpp::loglevel::ll_error, "Failed to execute insert guild " + std::to_string(guild_id) + " stmt! " + mln::database_handler::get_name_from_result(res) + ". " + delta()->db.get_last_err_msg());
    }
}