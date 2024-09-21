#include "commands/guild/create/insert_guild_db.h"
#include "database/database_handler.h"

#include <dpp/cluster.h>

#include <format>

mln::insert_guild_db::insert_guild_db(dpp::cluster& cluster, database_handler& in_db) :
    base_guild_create{ cluster }, db{in_db}, saved_insert_guild_query {}, valid_saved_insert{ true } {
    const mln::db_result_t res = db.save_statement("INSERT OR IGNORE INTO guild_profile(guild_id) VALUES(?);", saved_insert_guild_query);
    if (res.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert guild query statement! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
        valid_saved_insert = false;
    }
}

mln::insert_guild_db::~insert_guild_db()
{
    if (valid_saved_insert) {
        db.delete_statement(saved_insert_guild_query);
    }
}

dpp::task<void> mln::insert_guild_db::command(const dpp::guild_create_t& event_data) const {
    if (!valid_saved_insert) {
        bot().log(dpp::loglevel::ll_error, "Failed to insert guild in the db! The save insert stmt was not saved correctly.");
        co_return;
    }

    if (event_data.created == nullptr) {
        bot().log(dpp::loglevel::ll_error, "Failed to insert guild in the db! The created guild ptr is null!");
        co_return;
    }

    const int64_t guild_id = static_cast<int64_t>(event_data.created->id);

    mln::db_result_t res = db.bind_parameter(saved_insert_guild_query, 0, 1, guild_id);
    if (res.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to bind guild [{}] to insert guild stmt! Error: [{}], details: [{}].", guild_id, mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }
    
    res = db.exec(saved_insert_guild_query, mln::database_callbacks_t{});
    if (mln::database_handler::is_exec_error(res.type)) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to execute insert guild [{}] stmt! Error: [{}], details: [{}].", guild_id, mln::database_handler::get_name_from_result(res.type), res.err_text));
    }
}