#include "commands/guild/create/base_guild_create.h"
#include "commands/guild/create/insert_guild_db.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_result.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

#include <cstdint>
#include <format>
#include <functional>
#include <optional>

mln::insert_guild_db::insert_guild_db(dpp::cluster& cluster, database_handler& in_db) :
    base_guild_create{ cluster }, db{in_db}, saved_insert_guild_query {}, valid_saved_insert{ true } {
    const mln::db_result_t res = db.save_statement("INSERT OR IGNORE INTO guild_profile(guild_id) VALUES(?);", saved_insert_guild_query);
    if (res.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert guild query statement! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
        valid_saved_insert = false;
    }
    bot().log(dpp::loglevel::ll_debug, std::format("insert_guild_db: [{}].", valid_saved_insert));
}

mln::insert_guild_db::~insert_guild_db()
{
    if (valid_saved_insert) {
        db.delete_statement(saved_insert_guild_query);
    }
}

dpp::task<void> mln::insert_guild_db::command(const dpp::guild_create_t& event_data) const {
    bot().log(dpp::loglevel::ll_critical, "Failed to insert guild in the db! Usage of task command instead of job command.");
    co_return;
}

std::optional<std::function<void()>> mln::insert_guild_db::job(const dpp::guild_create_t& event_data) const
{
    if (!valid_saved_insert) {
        bot().log(dpp::loglevel::ll_error, "Failed to insert guild in the db! The save insert stmt was not saved correctly.");
        return std::nullopt;
    }

    const uint64_t created_guild_id = event_data.created ? static_cast<uint64_t>(event_data.created->id) : 0;

    if (created_guild_id == 0) {
        bot().log(dpp::loglevel::ll_error, "Failed to insert guild in the db! The created guild id is invalid!");
        return std::nullopt;
    }

    return [this, created_guild_id]() -> void {

        const int64_t guild_id = static_cast<int64_t>(created_guild_id);

        mln::db_result_t res = db.bind_parameter(saved_insert_guild_query, 0, 1, guild_id);
        if (res.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to bind guild [{}] to insert guild stmt! Error: [{}], details: [{}].", guild_id, mln::database_handler::get_name_from_result(res.type), res.err_text));
            return;
        }

        res = db.exec(saved_insert_guild_query, mln::database_callbacks_t{});
        if (mln::database_handler::is_exec_error(res.type)) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to execute insert guild [{}] stmt! Error: [{}], details: [{}].", guild_id, mln::database_handler::get_name_from_result(res.type), res.err_text));
        }
        };
}

bool mln::insert_guild_db::use_job() const
{
    return true;
}
