#include "commands/slash/db/db_update_dump_channel.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"

mln::db_update_dump_channel::db_update_dump_channel(bot_delta* const delta) : base_db_command(delta),
saved_stmt(), saved_param_guild(), saved_param_channel(), valid_stmt(true) {

    mln::db_result res1 = delta->db.save_statement("UPDATE OR ABORT guild_profile SET dedicated_channel_id = :CCC WHERE guild_id = :GGG RETURNING dedicated_channel_id;", saved_stmt);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_dump_channel stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":GGG", saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(saved_stmt, 0, ":CCC", saved_param_channel);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save update_dump_channel stmt param indexes!");
            valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_update_dump_channel::command(const dpp::command_data_option&, const dpp::slashcommand_t& event_data, url_type) {
    dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

    if (!valid_stmt) {
        co_await thinking;
        event_data.edit_response("Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    dpp::user command_usr = event_data.command.get_issuing_user();
    dpp::guild command_gld = event_data.command.get_guild();
    if (command_usr.id != delta()->dev_id && command_usr.id != command_gld.owner_id) {
        co_await thinking;
        event_data.edit_response("Failed to update the dump channel for this server, missing the required permission!");
        co_return;
    }

    dpp::snowflake channel_id;
    const dpp::command_value channel_param = event_data.get_parameter("channel");
    const bool valid_channel = std::holds_alternative<dpp::snowflake>(channel_param);
    if (valid_channel) {
        channel_id = std::get<dpp::snowflake>(channel_param);
    }

    mln::db_result res1 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_guild, static_cast<int64_t>(command_gld.id));
    mln::db_result res2;
    if (valid_channel) {
        res2 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_channel, static_cast<int64_t>(channel_id));
    } else {
        res2 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_channel);
    }

    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await thinking;
        event_data.edit_response("Failed to bind query params, internal error!");
        co_return;
    }

    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback();
    calls.callback_data = &db_success;

    dpp::message msg{};
    mln::db_result res = delta()->db.exec(saved_stmt, calls);
    if (res != mln::db_result::ok || !db_success) {
        msg.set_content(res == mln::db_result::ok && !db_success ? "Failed to update the dump channel, either no record found in the main database with the given guild id or you are not allowed to modify it!" : "Failed to update the dump channel, internal error!");
    } else {
        msg.set_content("Dump channel updated!");
        valid_channel ? delta()->update_dump_channels_cache(command_gld.id, channel_id) : delta()->update_dump_channels_cache(command_gld.id);
    }

    co_await thinking;
    event_data.edit_response(msg);
}
