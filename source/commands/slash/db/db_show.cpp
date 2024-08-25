#include "commands/slash/db/db_show.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "database/database_handler.h"

constexpr uint64_t s_paginated_embed_time_limit{180};

mln::db_show::db_show(bot_delta* const delta) : base_db_command(delta), data{.valid_stmt = true} {
    mln::db_result res1 = delta->db.save_statement("SELECT name, desc FROM storage WHERE guild_id = ? ORDER BY user_id ASC, creation_time ASC;", data.saved_stmt_all);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save show all stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    }

    res1 = delta->db.save_statement("SELECT name, desc FROM storage WHERE guild_id = :GGG AND user_id = :UUU ORDER BY creation_time ASC;", data.saved_stmt_user);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save show user stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_stmt_user, 0, ":GGG", data.saved_param_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_stmt_user, 0, ":UUU", data.saved_param_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save show user stmt param indexes!");
            data.valid_stmt = false;
        }
    }
}

dpp::task<void> mln::db_show::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const std::unordered_map<mln::db_command_type, std::function<dpp::task<void>(mln::db_show*, const dpp::slashcommand_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>> s_allowed_subcommands{
        {mln::db_command_type::all, &mln::db_show::all},
        {mln::db_command_type::help, &mln::db_show::help},
        {mln::db_command_type::user, &mln::db_show::user},
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

mln::db_init_type_flag mln::db_show::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::all, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::user, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::help, db_init_type_flag::cmd_data},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}
dpp::task<void> mln::db_show::execute_show(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, const exec_show_t& stmt_data, const std::optional<std::shared_ptr<std::vector<std::string>>>& cached_show) {
    const bool use_cache = cached_show.has_value();//TODO to make this work I need to delete the show element cached element (for both show all and show user) when inserting/deleting/updating descriptions, so that is gets refreshed next show performed
    
    std::shared_ptr<std::vector<std::string>> records;
    if (use_cache) {
        records = cached_show.value();
    }else{
        //Prepare callbacks for query execution
        records = std::make_shared<std::vector<std::string>>(std::vector<std::string>{});
        mln::database_callbacks_t calls{};
        calls.callback_data = static_cast<void*>(records.get());
        calls.type_definer_callback = [](void*, int) {return true;};
        calls.data_adder_callback = [](void* ptr, int c, mln::db_column_data_t&& d) {
            std::vector<std::string>* records = static_cast<std::vector<std::string>*>(ptr);
            if (c == 0) {
                records->push_back(reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data))));
                return;
            }

            const size_t index = records->size() - 1;
            (*records)[index] = "{[" + (*records)[index] + "], [" + reinterpret_cast<const char*>((std::get<const unsigned char*>(d.data))) + "]}\n";
        };

        //Execute query and return an error if the query failed or if no element was added
        mln::db_result res = delta()->db.exec(stmt_data.stmt, calls);
        if (res != mln::db_result::ok || records->empty()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, res == mln::db_result::ok && records->empty() ?
                "Failed while executing database query!" " No record found in the database!" :
                "Failed while executing database query! Internal error!");
            co_return;
        }

        //Update cache
        const bool all_stmt = stmt_data.stmt == data.saved_stmt_all;
        if (all_stmt) {
            delta()->show_all_cache.add_element(cmd_data.cmd_guild->id, std::shared_ptr(records));
        } else {
            delta()->show_user_cache.add_element({cmd_data.cmd_guild->id, stmt_data.target}, std::shared_ptr(records));
        }
    }

    //Create paginated embed job
    if (thinking.has_value()) {
        co_await thinking.value();
    }
    mln::utility::manage_paginated_embed(event_data, &(delta()->bot), records, s_paginated_embed_time_limit);
}

dpp::task<void> mln::db_show::all(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Bind query parameters
    mln::db_result res1 = delta()->db.bind_parameter(data.saved_stmt_all, 0, 1, static_cast<int64_t>(cmd_data.cmd_guild->id));

    //Check if any error occurred in the binding process, in case return an error
    if (res1 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to bind query parameters, internal error!");
        co_return;
    }

    co_await execute_show(event_data, cmd_data, thinking, exec_show_t{.stmt = data.saved_stmt_all, .target = 0}, delta()->show_all_cache.get_element(cmd_data.cmd_guild->id));
}

dpp::task<void> mln::db_show::user(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    //Bind query parameters
    const dpp::snowflake target = std::get<dpp::snowflake>(event_data.get_parameter("user"));
    mln::db_result res1 = delta()->db.bind_parameter(data.saved_stmt_user, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_stmt_user, 0, data.saved_param_user, static_cast<int64_t>(target));

    //Check if any error occurred in the binding process, in case return an error
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "Failed to bind query parameters, internal error!");
        co_return;
    }

    co_await execute_show(event_data, cmd_data, thinking, exec_show_t{.stmt = data.saved_stmt_user, .target = target}, delta()->show_user_cache.get_element({cmd_data.cmd_guild->id, target}));
}

dpp::task<void> mln::db_show::help(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db show` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(Work in progress...)"""));

    event_data.reply(s_info);
    co_return;
}
