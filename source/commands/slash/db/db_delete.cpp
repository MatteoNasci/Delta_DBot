#include "commands/slash/db/db_delete.h"
#include "utility/utility.h"
#include "utility/perms.h"
#include "utility/caches.h"
#include "utility/constants.h"
#include "database/database_handler.h"
#include "utility/response.h"
#include "utility/json_err.h"

#include <dpp/unicode_emoji.h>
#include <dpp/cluster.h>

#include <unordered_set>
#include <format>

static const uint64_t s_confirmation_button_timeout{ 60 };

const std::unordered_map<mln::db_command_type, std::tuple<
    mln::db_init_type_flag, 
    std::function<dpp::task<void>(const mln::db_delete&, const dpp::slashcommand_t&, const dpp::interaction_create_t&, const mln::db_cmd_data_t&)>, 
    std::string>> 
    mln::db_delete::s_mapped_commands_info{

    {mln::db_command_type::user, {mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking, &mln::db_delete::user, "Are you sure you want to delete ALL records related to the given user on this server? This action cannot be reversed."}},
    {mln::db_command_type::self, {mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking, &mln::db_delete::self, "Are you sure you want to delete ALL records related to your account from the entire database? All your records on all servers will be deleted. This action cannot be reversed."}},
    {mln::db_command_type::single, {mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking, &mln::db_delete::single, "Are you sure you want to delete the record identified by the given name on this server? This action cannot be reversed."}},
    {mln::db_command_type::guild, {mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking, &mln::db_delete::guild, "Are you sure you want to delete ALL records related to this server? This action cannot be reversed."}},
    {mln::db_command_type::help, {mln::db_init_type_flag::none, &mln::db_delete::help, ""}},
};

mln::db_delete::db_delete(dpp::cluster& cluster, database_handler& in_db) : base_db_command{ cluster }, data{ .valid_stmt = true }, db{ in_db } {
    //Delete a specific record by name, works only if used by record owner or by admin
    const mln::db_result_t res1 = db.save_statement("DELETE FROM storage WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING url;", data.saved_single);
    if (res1.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete single stmt! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        data.valid_stmt = false;
    } else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_single, 0, ":GGG", data.saved_param_single_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_single, 0, ":NNN", data.saved_param_single_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data.saved_single, 0, ":UUU", data.saved_param_single_user);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete single stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text));
            data.valid_stmt = false;
        }
    }

    //Delete all records of a specified user in the guild, works only if used by record owner or by admin
    const mln::db_result_t res2 = db.save_statement("DELETE FROM storage WHERE guild_id = :GGG AND user_id = :UUU RETURNING url;", data.saved_user);
    if (res2.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete user stmt! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        data.valid_stmt = false;
    } else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_user, 0, ":GGG", data.saved_param_user_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_user, 0, ":UUU", data.saved_param_user_user);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete user stmt param indexes! guild_param: [{}, {}], user_param: [{}, {}].", 
                mln::database_handler::get_name_from_result(res11.type), res11.err_text, 
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
            data.valid_stmt = false;
        }
    }

    //Delete all record of the guild, works only if used by admin
    const mln::db_result_t res3 = db.save_statement("DELETE FROM storage WHERE guild_id = ?1 RETURNING url, user_id;", data.saved_guild);
    if (res3.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete guild stmt! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        data.valid_stmt = false;
    }

    //Delete all records of a user (regardless of the server the records are saved in), works only for the command user
    const mln::db_result_t res4 = db.save_statement("DELETE FROM storage WHERE user_id = ?1 RETURNING url;", data.saved_self);
    if (res4.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete self stmt! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res4.type), res4.err_text));
        data.valid_stmt = false;
    }
}

dpp::task<void> mln::db_delete::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, const db_command_type type) const {
    //Find the command variant and execute it. If no valid command variant found return an error
    const bool is_first_reply = (mln::db_delete::get_requested_initialization_type(type) & mln::db_init_type_flag::thinking) == mln::db_init_type_flag::none;
    const auto it_func = s_mapped_commands_info.find(type);
    if (it_func == s_mapped_commands_info.end()) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data, 
            "Failed command, the given sub_command is not supported!"), bot(), &event_data, std::format("Failed command, the given sub_command [{}] is not supported for /db delete!", mln::get_cmd_type_text(type)));
        co_return;
    }

    //If the query statement was not saved correctly, return an error
    if (!data.valid_stmt) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data, 
            "Failed database operation, the database was not initialized correctly!"), bot(), &event_data, std::format("Failed database /db delete operation [{}], the database was not initialized correctly!", mln::get_cmd_type_text(type)));
        co_return;
    }

    dpp::button_click_t button_data{};
    if (!std::get<2>(it_func->second).empty()) {

        const std::string confirmation_button_id = std::format("{}{}", static_cast<uint64_t>(event_data.command.id), 'y');
        const std::string refuse_button_id = std::format("{}{}", static_cast<uint64_t>(event_data.command.id), 'n');

        dpp::message conf_msg{ std::get<2>(it_func->second) };
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


        if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(is_first_reply, event_data, conf_msg), bot())) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Failed to create confirmation prompt!"), bot(), &event_data, "Failed to respond with button confirmation prompt in delete command.");
            co_return;
        }

        const auto& result = co_await dpp::when_any{
        bot().co_sleep(s_confirmation_button_timeout),
        bot().on_button_click.when([&confirmation_button_id, &refuse_button_id](const dpp::button_click_t& event_data) {
            return event_data.custom_id == confirmation_button_id || event_data.custom_id == refuse_button_id;
            }) };

        //If the timer run out return an error
        if (result.index() == 0) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Too much time has passed since the last interaction, the command execution has terminated"), bot());
            co_return;
        }

        //If an exception occurred return an error
        if (result.is_exception()) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "An unknown error occurred!"), bot(), &event_data, "An exception was returned by ::when_any while attempting to get delete confirmation.");
            co_return;
        }

        //It was suggested to copy the event from documentation of ::when
        button_data = result.get<1>();
        dpp::async<dpp::confirmation_callback_t> thinking = button_data.co_thinking(true);

        if (button_data.custom_id == confirmation_button_id) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Proceeding with command execution..."), bot());
        }
        else if (button_data.custom_id == refuse_button_id) {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Command interrupted!"), bot());
            mln::utility::conf_callback_is_error(co_await thinking, bot());
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, button_data,
                "The command has been interrupted."), bot());
            co_return;
        }
        else {
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data,
                "Command interrupted!"), bot(), &event_data, "Command interrupted!");
            mln::utility::conf_callback_is_error(co_await thinking, bot());
            mln::utility::conf_callback_is_error(co_await mln::response::make_response(true, button_data,
                "Invalid button id found, internal error!"), bot(), &button_data, 
                std::format("Invalid button id found while waiting for delete confirmation! Found: [{}], expected: [{}] or [{}].",
                    button_data.custom_id, confirmation_button_id, refuse_button_id));

            co_return;
        }
        
        mln::utility::conf_callback_is_error(co_await thinking, bot());
    }

    co_await std::get<1>(it_func->second)(*(this), event_data,
        (button_data.command.id == 0 ? static_cast<dpp::interaction_create_t>(event_data) : static_cast<dpp::interaction_create_t>(button_data)), cmd_data);
}

mln::db_init_type_flag mln::db_delete::get_requested_initialization_type(const db_command_type cmd) const {

    const auto it = s_mapped_commands_info.find(cmd);
    if (it == s_mapped_commands_info.end()) {
        return mln::db_init_type_flag::all;
    }
    return std::get<0>(it->second);
}

dpp::task<void> mln::db_delete::single(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const {
    const dpp::command_value user_param = event_data.get_parameter("owner");
    dpp::snowflake target = cmd_data.cmd_usr->user_id;
    if (std::holds_alternative<dpp::snowflake>(user_param)) {
        target = std::get<dpp::snowflake>(user_param);
    }

    if (target != cmd_data.cmd_usr->user_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to delete the given user record, admin permission required to delete records owned by someone else!"), bot());
        co_return;
    }

    const std::string name = std::get<std::string>(event_data.get_parameter("name"));

    const mln::db_result_t res1 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_user, static_cast<int64_t>(target));
    const mln::db_result_t res3 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_name, name, mln::db_text_encoding::utf8);
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to bind query parameters, internal database error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], user_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_single, target);
}
dpp::task<void> mln::db_delete::user(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const {
    const dpp::command_value user_param = event_data.get_parameter("user");
    const dpp::snowflake target = std::get<dpp::snowflake>(user_param);
    
    if (target != cmd_data.cmd_usr->user_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to delete the given user records, admin permission required to delete records owned by someone else!"), bot());
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_user, 0, data.saved_param_user_guild, static_cast<int64_t>(cmd_data.cmd_guild->id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_user, 0, data.saved_param_user_user, static_cast<int64_t>(target));
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to bind query parameters, internal database error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_user, target);
}
dpp::task<void> mln::db_delete::guild(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const {
    if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to delete the guild records, admin permission required!"), bot());
        co_return;
    }

    if (cmd_data.cmd_guild->id != cmd_data.cmd_usr->guild_id) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to delete the guild records, mismatch found between the user guild id and the guild where the command was invoked!"), bot());
        co_return;
    }

    const mln::db_result_t res = db.bind_parameter(data.saved_guild, 0, 1, static_cast<int64_t>(cmd_data.cmd_guild->id));
    if (res.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to bind query parameters, internal database error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_guild, 0);
}
dpp::task<void> mln::db_delete::self(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const {

    const mln::db_result_t res = db.bind_parameter(data.saved_self, 0, 1, static_cast<int64_t>(cmd_data.cmd_usr->user_id));
    if (res.type != mln::db_result::ok) {
        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data,
            "Failed to bind query parameters, internal database error!"), bot(), &event_data, 
            std::format("Failed to bind query parameters, internal database error! user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res.type), res.err_text));
        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_self, cmd_data.cmd_usr->user_id);
}
struct db_delete_url_data_t {
    std::string url;
    uint64_t user;
};
struct db_delete_lists_data_t {
    std::vector<std::vector<dpp::snowflake>> lists;
};
dpp::task<void> mln::db_delete::exec(const dpp::slashcommand_t& event_data, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data, const size_t stmt, const uint64_t target) const {
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

    const mln::db_result_t res = db.exec(stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || urls.empty()) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && urls.empty();
        const std::string err_text = is_user_error ? 
            "Failed while executing database query! Either no records found or you don't have the ownership over the records to delete!" :
            "Failed while executing database query! Internal database error!";

        mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data, err_text), bot(), &event_data, 
            std::format("{} Error: [{}], err_text: [{}]",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

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
            bot().log(dpp::loglevel::ll_warning, "Failed to retrieve data from url for delete messages.");
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

        const reply_log_data_t reply_log_data{ &event_data, &bot(), false };
        const auto it3 = visited_guild_by_channel.find({ url_guild, url_channel });
        if (it3 == visited_guild_by_channel.end()) {
            visited_guild_by_channel.insert({ url_guild, url_channel });

            //Retrieve guild data
            const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_full(url_guild, reply_log_data);
            if (!guild.has_value()) {
                co_return;
            }

            //Retrieve channel data
            const std::optional<std::shared_ptr<const dpp::channel>> channel = co_await mln::caches::get_channel_full(url_channel, reply_log_data);
            if (!channel.has_value()) {
                co_return;
            }

            //Retrieve bot information
            const std::optional<std::shared_ptr<const dpp::guild_member>> bot_opt = co_await mln::caches::get_member_full({ url_guild, reply_data.command.application_id }, reply_log_data);
            if (!bot_opt.has_value()) {
                co_return;
            }

            const std::optional<dpp::permission> bot_perm = co_await mln::perms::get_computed_permission_full(*(guild.value()), *(channel.value()), *(bot_opt.value()), reply_log_data);
            if (!bot_perm.has_value()) {
                co_return;
            }

            if (!mln::perms::check_permissions(bot_perm.value(), dpp::permissions::p_view_channel | dpp::permissions::p_read_message_history)) {
                ++failed_deletes;
                bot().log(dpp::loglevel::ll_warning, std::format("The bot doesn't have the permission to delete bulk a message! Guild: [{}], channel: [{}].", url_guild, url_channel));
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

            if (list.size() >= mln::constants::get_min_msg_bulk_delete()) {
                const dpp::confirmation_callback_t bulk_conf = co_await bot().co_message_delete_bulk(list, map_pair.first);
                if (bulk_conf.is_error()) {
                    const dpp::error_info err = bulk_conf.get_error();

                    if (err.code == static_cast<std::underlying_type<mln::json_err>::type>(mln::json_err::message_too_old_bulk_delete)) {
                        for (const dpp::snowflake& msg_to_delete : list) {
                            if (mln::utility::conf_callback_is_error(co_await bot().co_message_delete(msg_to_delete, map_pair.first), bot(), nullptr,
                                std::format("Failed to delete message [{}] from channel [{}]!", static_cast<uint64_t>(msg_to_delete), map_pair.first))) {

                                ++failed_deletes;
                            }
                        }
                    }
                    else {
                        mln::utility::create_event_log_error(event_data, bot(), std::format("Failed to delete bulk messages from channel: [{}]! Error: [{}], details: [{}].", 
                            map_pair.first, mln::get_json_err_text(err.code), err.human_readable));

                        failed_deletes += list.size();
                    }
                }
            }
            else {
                if (mln::utility::conf_callback_is_error(co_await bot().co_message_delete(list[0], map_pair.first), bot(), nullptr,
                    std::format("Failed to delete message [{}] from channel [{}]!", static_cast<uint64_t>(list[0]), map_pair.first))) {

                    ++failed_deletes;
                }
            }
        }
    }

    const bool error = failed_deletes != 0 || malformed_urls != 0;
    const std::string final_text = error ?
        std::format("Command concluded with some errors, failed to remove all the stored messages related to the deleted records! Malformed urls: [{}], failed deletes: [{}], total deleted: [{}].", 
            malformed_urls, failed_deletes, (total_urls_to_delete - malformed_urls - failed_deletes)) :
        "Command executed!";

    mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, reply_data, final_text), bot());
    if (error) {
        mln::utility::create_event_log_error(event_data, bot(), final_text);
    }

    co_return;
}

dpp::task<void> mln::db_delete::help(const dpp::slashcommand_t&, const dpp::interaction_create_t& reply_data, const db_cmd_data_t& cmd_data) const {
    static const dpp::message s_info = dpp::message{ "Information regarding the `/db delete` commands..." }
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

    if (mln::utility::conf_callback_is_error(co_await reply_data.co_reply(s_info), bot())) {
        mln::utility::create_event_log_error(reply_data, bot(), "Failed to reply with the db delete help text!");
    }
    co_return;
}
