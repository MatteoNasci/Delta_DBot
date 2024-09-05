#include "commands/slash/db/db_update_dump_channel.h"
#include "database/database_handler.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/caches.h"
#include "utility/perms.h"

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

mln::db_init_type_flag mln::db_update_dump_channel::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::update_dump_channel, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::help, db_init_type_flag::none},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}

dpp::task<void> mln::db_update_dump_channel::update_dump(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    dpp::snowflake channel_id{0};
    const dpp::command_value channel_param = event_data.get_parameter("channel");
    if (std::holds_alternative<dpp::snowflake>(channel_param)) {
        channel_id = std::get<dpp::snowflake>(channel_param);
    }

    std::optional<uint64_t> opt_channel = mln::caches::dump_channels_cache.get_element(cmd_data.cmd_guild->id);

    if (opt_channel.has_value()) {
        if (opt_channel.value() == channel_id) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Channel found in cache. The given channel_id is already set as the dump channel!", {false, dpp::loglevel::ll_debug});
            co_return;
        }
    }

    mln::db_result res1 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(saved_stmt, 0, saved_param_channel, static_cast<int64_t>(channel_id));

    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to bind query params, internal error!");
        co_return;
    }

    bool db_success = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

    std::string msg{};
    mln::db_result res = delta()->db.exec(saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res) || !db_success) {
        msg = (!mln::database_handler::is_exec_error(res) && !db_success ? "Failed to update the dump channel, either no record found in the main database with the given guild id or you are not allowed to modify it!" : "Failed to update the dump channel, internal error!");
        mln::caches::dump_channels_cache.remove_element(cmd_data.cmd_guild->id);
    } else {
        msg = ("Dump channel updated!");
        mln::caches::dump_channels_cache.add_element(cmd_data.cmd_guild->id, channel_id);
    }
    co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, std::move(msg), {false, dpp::loglevel::ll_debug});
}

dpp::task<void> mln::db_update_dump_channel::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db setup` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db setup` set of commands is used to configure the database environment, potentially altering the behavior of all other commands.

This set of commands is generally reserved for users with specific permissions.

**Types of setup:**

- **/db setup update_dump_channel**  
  *Parameters:* channel[text_channel, optional].  
  This command asks for a valid text channel to be used for storage purposes. If not provided, the current dump channel will be unset, and the database will use the channel where the database commands are used as the dump channel.  
  To avoid clutter, it is recommended to set a specific dump channel. The other database commands will use this channel to store data and records.  
  Ideally, this channel should be reserved for the bot’s use, and the messages created by the bot should not be edited or deleted, as this may cause the database to have broken records linking to content that has been modified or removed.)"""));

    event_data.reply(s_info);
    co_return;
}

dpp::task<void> mln::db_update_dump_channel::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const std::unordered_map<mln::db_command_type, std::function<dpp::task<void>(mln::db_update_dump_channel*, const dpp::slashcommand_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>> s_allowed_subcommands{
        {mln::db_command_type::update_dump_channel, &mln::db_update_dump_channel::update_dump},
        {mln::db_command_type::help, &mln::db_update_dump_channel::help},
    };

    //Find the command variant and execute it. If no valid command variant found return an error
    const auto it_func = s_allowed_subcommands.find(type);
    if (it_func == s_allowed_subcommands.end()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
            "Failed command, the given sub_command is not supported!");
        co_return;
    }

    if (cmd_data.cmd_usr && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed database operation, admin permission is required to access this group command!");
        co_return;
    }
    
    if (!valid_stmt) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed database operation, the database was not initialized correctly!");
        co_return;
    }

    co_await it_func->second(this, event_data, cmd_data, thinking);
}
