#include "commands/slash/db/db_delete.h"
#include "bot_delta.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "utility/constants.h"
#include "database/database_handler.h"

#include <dpp/unicode_emoji.h>

#include <unordered_set>

static const uint64_t s_confirmation_button_timeout{ 60 };

mln::db_delete::db_delete(bot_delta* const delta) : base_db_command(delta), data{.valid_stmt = true} {
    //Delete a specific record by name, works only if used by record owner or by admin
    mln::db_result res1 = delta->db.save_statement("DELETE FROM storage WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING url;", data.saved_single);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_single, 0, ":GGG", data.saved_param_single_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_single, 0, ":NNN", data.saved_param_single_name);
        mln::db_result res13 = delta->db.get_bind_parameter_index(data.saved_single, 0, ":UUU", data.saved_param_single_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok || res13 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt param indexes!");
            data.valid_stmt = false;
        }
    }

    //Delete all records of a specified user in the guild, works only if used by record owner or by admin
    res1 = delta->db.save_statement("DELETE FROM storage WHERE guild_id = :GGG AND user_id = :UUU RETURNING url;", data.saved_user);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    } else {
        mln::db_result res11 = delta->db.get_bind_parameter_index(data.saved_user, 0, ":GGG", data.saved_param_user_guild);
        mln::db_result res12 = delta->db.get_bind_parameter_index(data.saved_user, 0, ":UUU", data.saved_param_user_user);
        if (res11 != mln::db_result::ok || res12 != mln::db_result::ok) {
            delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt param indexes!");
            data.valid_stmt = false;
        }
    }

    //Delete all record of the guild, works only if used by admin
    res1 = delta->db.save_statement("DELETE FROM storage WHERE guild_id = ?1 RETURNING url, user_id;", data.saved_guild);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    }

    //Delete all records of a user (regardless of the server the records are saved in), works only for the command user
    res1 = delta->db.save_statement("DELETE FROM storage WHERE user_id = ?1 RETURNING url;", data.saved_self);
    if (res1 != mln::db_result::ok) {
        delta->bot.log(dpp::loglevel::ll_error, "Failed to save delete stmt! " + mln::database_handler::get_name_from_result(res1) + ", " + delta->db.get_last_err_msg());
        data.valid_stmt = false;
    }
}

dpp::task<void> mln::db_delete::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const std::unordered_map<mln::db_command_type, 
        std::tuple<std::function<dpp::task<void>(mln::db_delete*, const dpp::slashcommand_t&, const dpp::interaction_create_t&, const db_cmd_data_t&, std::optional<dpp::async<dpp::confirmation_callback_t>>&)>, std::string>
    > s_allowed_subcommands{
        {db_command_type::user, {&mln::db_delete::user, "Are you sure you want to delete ALL records related to the given user on this server? This action cannot be reversed."}},
        {db_command_type::self, {&mln::db_delete::self, "Are you sure you want to delete ALL records related to your account from the entire database? All your records on all servers will be deleted. This action cannot be reversed."}},
        {db_command_type::single, {&mln::db_delete::single, "Are you sure you want to delete the record identified by the given name on this server? This action cannot be reversed."}},
        {db_command_type::guild, {&mln::db_delete::guild, "Are you sure you want to delete ALL records related to this server? This action cannot be reversed."}},
        {db_command_type::help, {&mln::db_delete::help, ""}},
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

    dpp::button_click_t button_data{};
    std::optional<dpp::async<dpp::confirmation_callback_t>> new_thinking{std::nullopt};
    if (!std::get<1>(it_func->second).empty()) {

        const std::string confirmation_button_id = std::to_string(event_data.command.id) + "y";
        const std::string refuse_button_id = std::to_string(event_data.command.id) + "n";

        dpp::message conf_msg{ std::get<1>(it_func->second) };
        conf_msg.set_flags(dpp::m_ephemeral).add_component(
                dpp::component{}.set_type(dpp::component_type::cot_action_row)
                .add_component(
                    dpp::component{}
                    .set_label("cancel")
                    .set_type(dpp::component_type::cot_button)
                    .set_emoji(dpp::unicode_emoji::cross_mark)
                    .set_style(dpp::component_style::cos_primary)
                    .set_id(refuse_button_id))
                .add_component(
                    dpp::component{}
                    .set_label("confirm")
                    .set_type(dpp::component_type::cot_button)
                    .set_emoji(dpp::unicode_emoji::check_mark)
                    .set_style(dpp::component_style::cos_success)
                    .set_id(confirmation_button_id)));

        if (thinking.has_value()) {
            co_await thinking.value();
            thinking = std::nullopt;
        }
        event_data.edit_response(conf_msg);

        const auto& result = co_await dpp::when_any{
            delta()->bot.co_sleep(s_confirmation_button_timeout),
            delta()->bot.on_button_click.when([&confirmation_button_id, &refuse_button_id](const dpp::button_click_t& event_data) {
                return event_data.custom_id == confirmation_button_id || event_data.custom_id == refuse_button_id;
                }) };

        //If the timer run out return an error
        if (result.index() == 0) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, 
                "Too much time has passed since the last interaction, the command execution has terminated", {true, dpp::loglevel::ll_error}, false);
            co_return;
        }

        //If an exception occurred return an error
        if (result.is_exception()) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "An unknown error occurred!", { true, dpp::loglevel::ll_error }, false);
            co_return;
        }

        //It was suggested to copy the event from documentation of ::when
        button_data = result.get<1>();
        if (button_data.cancelled) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot, "The button event was cancelled!", { true, dpp::loglevel::ll_error }, false);
            co_return;
        }

        //co_await button_data.co_reply(dpp::message{"Button event received"}.set_flags(dpp::m_ephemeral));
        new_thinking = button_data.co_thinking(true);

        if (button_data.custom_id == confirmation_button_id) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Proceeding with command execution...", { false, dpp::loglevel::ll_debug }, false);
        }
        else if (button_data.custom_id == refuse_button_id) {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Command interrupted!", { false, dpp::loglevel::ll_debug }, false);
            co_await mln::utility::co_conclude_thinking_response(new_thinking, button_data, delta()->bot,
                "The command has been interrupted.", { false, dpp::loglevel::ll_debug });
            co_return;
        }
        else {
            co_await mln::utility::co_conclude_thinking_response(thinking, event_data, delta()->bot,
                "Command interrupted!", { false, dpp::loglevel::ll_debug }, false);
            co_await mln::utility::co_conclude_thinking_response(new_thinking, button_data, delta()->bot,
                "Invalid button id found, internal error!");
            co_return;
        }
    }

    co_await std::get<0>(it_func->second)(this, event_data,
        (button_data.command.id == 0 ? static_cast<dpp::interaction_create_t>(event_data) : static_cast<dpp::interaction_create_t>(button_data)), cmd_data, 
        new_thinking.has_value() ? new_thinking : thinking);
}

mln::db_init_type_flag mln::db_delete::get_requested_initialization_type(db_command_type cmd) {
    static const std::unordered_map<db_command_type, db_init_type_flag> s_mapped_initialization_types{
        {db_command_type::user, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::self, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::single, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::guild, db_init_type_flag::cmd_data | db_init_type_flag::thinking},
        {db_command_type::help, db_init_type_flag::none},
    };

    const auto it = s_mapped_initialization_types.find(cmd);
    if (it == s_mapped_initialization_types.end()) {
        return mln::db_init_type_flag::all;
    }
    return it->second;
}

dpp::task<void> mln::db_delete::single(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    const dpp::command_value user_param = event_data.get_parameter("owner");
    dpp::snowflake target = cmd_data.cmd_usr->user_id;
    if (std::holds_alternative<dpp::snowflake>(user_param)) {
        target = std::get<dpp::snowflake>(user_param);
    }

    if (target != cmd_data.cmd_usr->user_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to delete the given user record, admin permission required to delete records owned by someone else!");
        co_return;
    }

    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    mln::db_result res1 = delta()->db.bind_parameter(data.saved_single, 0, data.saved_param_single_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_single, 0, data.saved_param_single_user, static_cast<int64_t>(target));
    mln::db_result res3 = delta()->db.bind_parameter(data.saved_single, 0, data.saved_param_single_name, name, mln::db_text_encoding::utf8);
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to bind query parameters, internal error! " + delta()->db.get_last_err_msg());
        co_return;
    }

    co_await mln::db_delete::exec(reply_data, cmd_data, thinking, data.saved_single, target);
}
dpp::task<void> mln::db_delete::user(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    const dpp::command_value user_param = event_data.get_parameter("user");
    dpp::snowflake target = std::get<dpp::snowflake>(user_param);
    
    if (target != cmd_data.cmd_usr->user_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to delete the given user records, admin permission required to delete records owned by someone else!");
        co_return;
    }

    mln::db_result res1 = delta()->db.bind_parameter(data.saved_user, 0, data.saved_param_user_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    mln::db_result res2 = delta()->db.bind_parameter(data.saved_user, 0, data.saved_param_user_user, static_cast<int64_t>(target));
    if (res1 != mln::db_result::ok || res2 != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to bind query parameters, internal error! " + delta()->db.get_last_err_msg());
        co_return;
    }

    co_await mln::db_delete::exec(reply_data, cmd_data, thinking, data.saved_user, target);
}
dpp::task<void> mln::db_delete::guild(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to delete the guild records, admin permission required!");
        co_return;
    }

    if (cmd_data.cmd_guild->id != cmd_data.cmd_usr->guild_id) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to delete the guild records, mismatch found between the user guild id and the guild where the command was invoked!");
        co_return;
    }

    mln::db_result res = delta()->db.bind_parameter(data.saved_guild, 0, 1, static_cast<int64_t>(cmd_data.cmd_guild->id));
    if (res != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to bind query parameters, internal error! " + delta()->db.get_last_err_msg());
        co_return;
    }

    co_await mln::db_delete::exec(reply_data, cmd_data, thinking, data.saved_guild, 0);
}
dpp::task<void> mln::db_delete::self(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {

    mln::db_result res = delta()->db.bind_parameter(data.saved_self, 0, 1, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    if (res != mln::db_result::ok) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot,
            "Failed to bind query parameters, internal error! " + delta()->db.get_last_err_msg());
        co_return;
    }

    co_await mln::db_delete::exec(reply_data, cmd_data, thinking, data.saved_self, cmd_data.cmd_usr->user_id);
}
//TODO add changelog as command
struct db_delete_url_data_t {
    std::string url;
    uint64_t user;
};
struct db_delete_lists_data_t {
    std::vector<std::vector<dpp::snowflake>> lists;
};
dpp::task<void> mln::db_delete::exec(const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, size_t stmt, uint64_t target) {
    //Extract all deleted urls and attempt to delete the original messages
    std::vector<db_delete_url_data_t> urls{};
    mln::database_callbacks_t calls{};
    calls.type_definer_callback = [](void*, int c) {return c == 0; };
    calls.data_adder_callback = [&urls](void*, int c, mln::db_column_data_t&& d) {
        if (c == 0) {
            urls.emplace_back(reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data)), 0);
            return;
        }

        const size_t index = urls.size() - 1;
        urls[index].user = static_cast<uint64_t>(std::get<int64_t>(d.data));
        };

    mln::db_result res = delta()->db.exec(stmt, calls);
    if (mln::database_handler::is_exec_error(res) || urls.empty()) {
        co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, (!mln::database_handler::is_exec_error(res) || res == mln::db_result::constraint_primary_key) && urls.empty() ?
            "Failed while executing database query! Either no records found or you don't have the ownership over the records to delete!" :
            "Failed while executing database query! Internal error! " + mln::database_handler::get_name_from_result(res));
        co_return;
    }

    const size_t total_urls_to_delete = urls.size();
    std::unordered_set<uint64_t> visited_guilds{};
    std::unordered_set<std::tuple<uint64_t, uint64_t>, mln::caches::composite_tuple_hash> visited_guild_by_user{};
    std::unordered_set<std::tuple<uint64_t, uint64_t>, mln::caches::composite_tuple_hash> visited_guild_by_channel{};
    std::map<uint64_t, db_delete_lists_data_t> messages_per_channel_map{};
    size_t malformed_urls = 0;
    size_t failed_deletes = 0;
    for (const db_delete_url_data_t& url : urls) {
        uint64_t url_guild{}, url_channel{}, url_message{};
        if (!mln::utility::extract_message_url_data(url.url, url_guild, url_channel, url_message) || url_channel == 0 || url_message == 0 || url_guild == 0) {
            ++malformed_urls;
            delta()->bot.log(dpp::loglevel::ll_warning, "Failed to retrieve data from url for delete bulk messages.");
            continue;
        }

        db_delete_lists_data_t& lists = messages_per_channel_map[url_channel];
        if (lists.lists.empty()) {
            lists.lists.emplace_back(std::vector<dpp::snowflake>{});
        }

        const auto it = visited_guilds.find(url_guild);
        if (it == visited_guilds.end()) {
            visited_guilds.insert(url_guild);

            mln::caches::show_all_cache.remove_element(url_guild);
        }

        const uint64_t actual_target = url.user != 0 ? url.user : target;
        if (actual_target != 0) {
            const auto it2 = visited_guild_by_user.find({ url_guild, actual_target });
            if (it2 == visited_guild_by_user.end()) {
                visited_guild_by_user.insert({ url_guild, actual_target });

                mln::caches::show_user_cache.remove_element({ url_guild, actual_target });
            }
        }

        const auto it3 = visited_guild_by_channel.find({ url_guild, url_channel });
        if (it3 == visited_guild_by_channel.end()) {
            visited_guild_by_channel.insert({ url_guild, url_channel });

            //Retrieve guild data
            std::optional<std::shared_ptr<const dpp::guild>> guild = mln::caches::get_guild(url_guild);
            if (!guild.has_value()) {
                guild = co_await mln::caches::get_guild_task(url_guild);
                if (!guild.has_value()) {
                    //Error can't find guild
                    co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, "Failed to retrieve guild data! guild_id: "
                        + std::to_string(url_guild));
                    co_return;
                }
            }

            //Retrieve channel data
            std::optional<std::shared_ptr<const dpp::channel>> channel = mln::caches::get_channel(url_channel, &reply_data);
            if (!channel.has_value()) {
                channel = co_await mln::caches::get_channel_task(url_channel);
                if (!channel.has_value()) {
                    //Error can't find channel
                    co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, "Failed to retrieve channel data! channel_id: "
                        + std::to_string(url_channel));
                    co_return;
                }
            }

            //Retrieve bot information
            std::optional<std::shared_ptr<const dpp::guild_member>> bot = mln::caches::get_member({ url_guild, reply_data.command.application_id }, &reply_data);
            if (!bot.has_value()) {
                bot = co_await mln::caches::get_member_task({ url_guild, reply_data.command.application_id });
                if (!bot.has_value()) {
                    //Error can't find bot
                    co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, "Failed to retrieve command bot data! bot id: "
                        + std::to_string(reply_data.command.application_id));
                    co_return;
                }
            }

            std::optional<dpp::permission> bot_perm = mln::perms::get_computed_permission(*(guild.value()), *(channel.value()), *(bot.value()), &reply_data);
            if (!bot_perm.has_value()) {
                bot_perm = co_await mln::perms::get_computed_permission_task(*(guild.value()), *(channel.value()), *(bot.value()), &reply_data);
                if (!bot_perm.has_value()) {
                    co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, "Failed to retrieve bot permission data! bot id: "
                        + std::to_string(bot.value()->user_id));
                    co_return;
                }
            }

            if (!mln::perms::check_permissions(bot_perm.value(), dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history)) {
                ++failed_deletes;
                delta()->bot.log(dpp::loglevel::ll_warning, "The bot doesn't have the permission to delete bulk a message! Guild: " + std::to_string(url_guild) + ", channel: " + std::to_string(url_channel));
                continue;
            }
        }

        for (size_t i = lists.lists.size() - 1; i < lists.lists.size(); ++i) {
            std::vector<dpp::snowflake>& list = lists.lists[i];

            if (list.size() < mln::constants::get_max_msg_bulk_delete()) {
                list.emplace_back(url_message);
                break;
            }
            else {
                lists.lists.emplace_back(std::vector<dpp::snowflake>{});
            }
        }
    }

    for (const std::pair<uint64_t, db_delete_lists_data_t>& map_pair : messages_per_channel_map) {
        if (map_pair.second.lists.size() == 0) {
            continue;
        }

        for (const std::vector<dpp::snowflake>& list : map_pair.second.lists) {
            if (list.size() == 0) {
                continue;
            }

            dpp::confirmation_callback_t result{};
            if (list.size() >= mln::constants::get_min_msg_bulk_delete()) {
                result = co_await delta()->bot.co_message_delete_bulk(list, map_pair.first);
            }
            else {
                result = co_await delta()->bot.co_message_delete(list[0], map_pair.first);
            }

            if (result.is_error()) {
                ++failed_deletes;
                delta()->bot.log(dpp::loglevel::ll_warning, "Failed to delete messages from channel: " + std::to_string(map_pair.first) + ", error: " + result.get_error().human_readable);
            }
        }
    }

    const bool error = failed_deletes != 0 || malformed_urls != 0;
    co_await mln::utility::co_conclude_thinking_response(thinking, reply_data, delta()->bot, error ?
        "Command concluded with some errors, failed to remove all the stored messages related to the deleted records! malformed urls: " + std::to_string(malformed_urls) + ", failed dels: " + std::to_string(failed_deletes) + ", total urls: " + std::to_string(total_urls_to_delete) :
        "Command executed!", { false, dpp::loglevel::ll_debug });
    co_return;
}

dpp::task<void> mln::db_delete::help(const dpp::slashcommand_t&, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking) {
    static const dpp::message s_info = dpp::message{"Information regarding the `/db delete` commands..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/db delete` set of commands is used to delete one or more records from the database. 

Once the records are removed from the database, the bot will attempt to delete the underlying messages associated with those records. However, this may not always succeed due to the bot no longer having permission to modify the stored message or channel, or for other reasons beyond the bot's control. The user will be notified if one or more messages could not be deleted.

Only records owned by the command user can be deleted. An error will be displayed if an attempt is made to delete records without ownership.

A user owns a record if it was created by them (through the `/db insert` commands). Admins can override this restriction, allowing them to delete all records in a Discord server regardless of ownership.

For all `/db delete` commands, confirmation is required before proceeding. Please read the warning carefully, as all deletions are permanent and cannot be reversed.

**Types of delete:**

- **/db delete single**  
  *Parameters:* name[text, required], owner[user ID, optional].  
  This command will ask you to provide the name of the record to delete and, optionally, the user ID of the record's owner. The user ID must be provided if you are attempting to delete a record owned by someone else, even if you have admin permissions.
  If the owner is not provided, the bot will assume the record to delete belongs to the command user. If this assumption is incorrect, an error will occur.
  Once the user confirms the delete command, the bot will verify the user's permissions and then proceed to delete the record identified by the given name from the local Discord server database.
  If no error occurs, the bot will also attempt to remove the associated stored message (not guaranteed).

- **/db delete user**  
  *Parameters:* user[user ID, required].  
  This command will ask you to provide the user ID of the record owner. The command will delete all records created by the specified user.
  Once the user confirms the delete command, the bot will verify the user's permissions and then proceed to delete the records owned by the given user from the local Discord server database.
  If no error occurs, the bot will also attempt to remove the associated stored messages (not guaranteed).

- **/db delete guild**  
  *Parameters:* none.  
  Once the user confirms the delete command, the bot will verify the user's permissions and then proceed to delete ALL records present in the local Discord server database. Admin permissions are required to perform this command.
  If no error occurs, the bot will also attempt to remove the associated stored messages (not guaranteed).

- **/db delete self**  
  *Parameters:* none.  
  Once the user confirms the delete command, the bot will proceed to delete ALL records owned by the command user in the entire database (including the local Discord server database and all other server databases).
  If no error occurs, the bot will also attempt to remove the associated stored messages (not guaranteed).)"""));

    reply_data.reply(dpp::message{s_info});
    co_return;
}
