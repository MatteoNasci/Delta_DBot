#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_delete.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_column_data.h"
#include "database/db_result.h"
#include "database/db_text_encoding.h"
#include "utility/caches.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/json_err.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/coro/when_any.h>
#include <dpp/dispatcher.h>
#include <dpp/event_router.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/snowflake.h>
#include <dpp/unicode_emoji.h>

#include <cstdint>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

static const uint64_t s_confirmation_button_timeout{ 60 };

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

dpp::task<void> mln::db_delete::command(const dpp::slashcommand_t& event_data, db_cmd_data_t& cmd_data, const db_command_type type) const {

    const std::string button_text = mln::db_delete::get_warning_message(type);
    event_data_lite_t button_lite{};
    if (!button_text.empty()) {

        const std::string confirmation_button_id = std::format("{}{}", static_cast<uint64_t>(cmd_data.data.command_id), 'y');
        const std::string refuse_button_id = std::format("{}{}", static_cast<uint64_t>(cmd_data.data.command_id), 'n');

        dpp::message conf_msg{ button_text };
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


        if (co_await mln::response::co_respond(cmd_data.data, conf_msg, false, "Failed to respond with button confirmation prompt in delete command.")) {
            co_await mln::response::co_respond(cmd_data.data, "Failed to create confirmation prompt!", false, {});
            co_return;
        }

        const auto& result = co_await dpp::when_any{
        bot().co_sleep(s_confirmation_button_timeout),
        bot().on_button_click.when([&confirmation_button_id, &refuse_button_id](const dpp::button_click_t& event_data) {
            return event_data.custom_id == confirmation_button_id || event_data.custom_id == refuse_button_id;
            }) };

        //If the timer run out return an error
        if (result.index() == 0) {
            co_await mln::response::co_respond(cmd_data.data, "Too much time has passed since the last interaction, the command execution has terminated", false, {});
            co_return;
        }

        //If an exception occurred return an error
        if (result.is_exception()) {
            co_await mln::response::co_respond(cmd_data.data, "An unknown error occurred!", true, "An exception was returned by ::when_any while attempting to get delete confirmation.");
            co_return;
        }

        //It was suggested to copy the event from documentation of ::when
        dpp::button_click_t button_data = result.get<1>();
        button_lite = event_data_lite_t{button_data, bot(), true};
        if (!mln::response::is_event_data_valid(button_lite)) {
            co_await mln::response::co_respond(button_lite, "Failed db command, the button event is incorrect!", true, "Failed db command, the button event is incorrect!");
            co_return;
        }

        co_await mln::response::co_think(button_lite, true, false, {});

        if (button_data.custom_id == confirmation_button_id) {
            co_await mln::response::co_respond(cmd_data.data, "Proceeding with command execution...", false, {});
        }
        else if (button_data.custom_id == refuse_button_id) {
            co_await mln::response::co_respond(cmd_data.data, "Command interrupted!", false, {});
            co_await mln::response::co_respond(button_lite, "The command has been interrupted.", false, {});
            co_return;
        }
        else {
            co_await mln::response::co_respond(cmd_data.data, "Command interrupted!", false, {});
            co_await mln::response::co_respond(button_lite, "Invalid button id found, internal error!", true,
                std::format("Invalid button id found while waiting for delete confirmation! Found: [{}], expected: [{}] or [{}].",
                    button_data.custom_id, confirmation_button_id, refuse_button_id));

            co_return;
        }
    }

    switch (type) {
    case mln::db_command_type::user:
        co_await mln::db_delete::user(event_data, button_lite.command_id ? button_lite : cmd_data.data, cmd_data);
        break;
    case mln::db_command_type::self:
        co_await mln::db_delete::self(event_data, button_lite.command_id ? button_lite : cmd_data.data, cmd_data);
        break;
    case mln::db_command_type::single:
        co_await mln::db_delete::single(event_data, button_lite.command_id ? button_lite : cmd_data.data, cmd_data);
        break;
    case mln::db_command_type::guild:
        co_await mln::db_delete::guild(event_data, button_lite.command_id ? button_lite : cmd_data.data, cmd_data);
        break;
    case mln::db_command_type::help:
        co_await mln::db_delete::help(cmd_data);
        break;
    default:
        co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
            std::format("Failed command, the given sub_command [{}] is not supported for /db delete!", mln::get_cmd_type_text(type)));
        break;
    }
}

mln::db_init_type_flag mln::db_delete::get_requested_initialization_type(const db_command_type cmd) const {
    switch (cmd) {
    case mln::db_command_type::user:
    case mln::db_command_type::self:
    case mln::db_command_type::single:
    case mln::db_command_type::guild:
        return mln::db_init_type_flag::cmd_data | mln::db_init_type_flag::thinking;
    case mln::db_command_type::help:
        return db_init_type_flag::none;
    default:
        return mln::db_init_type_flag::all;
    }
}

std::string mln::db_delete::get_warning_message(const db_command_type type) const {
    switch (type) {
    case mln::db_command_type::user:
        return "Are you sure you want to delete ALL records related to the given user on this server? This action cannot be reversed.";
    case mln::db_command_type::self:
        return "Are you sure you want to delete ALL records related to your account from the entire database? All your records on all servers will be deleted. This action cannot be reversed.";
    case mln::db_command_type::single:
        return "Are you sure you want to delete the record identified by the given name on this server? This action cannot be reversed.";
    case mln::db_command_type::guild:
        return "Are you sure you want to delete ALL records related to this server? This action cannot be reversed.";
    case mln::db_command_type::help:
    default:
        return "";
    }
}
bool mln::db_delete::is_db_initialized() const
{
    return data.valid_stmt;
}

dpp::task<void> mln::db_delete::single(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const {
    const dpp::command_value& user_param = event_data.get_parameter("owner");
    dpp::snowflake target = cmd_data.data.usr_id;
    if (std::holds_alternative<dpp::snowflake>(user_param)) {
        target = std::get<dpp::snowflake>(user_param);
    }

    if (target != cmd_data.data.usr_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(reply_data, "Failed to delete the given user record, admin permission required to delete records owned by someone else!", false, {});
        co_return;
    }

    const dpp::command_value& name_param = event_data.get_parameter("name");
    const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};

    if (name.empty()) {
        co_await mln::response::co_respond(reply_data, "Failed to retrieve name parameter!", true, "Failed to retrieve name parameter!");
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_user, static_cast<int64_t>(target));
    const mln::db_result_t res3 = db.bind_parameter(data.saved_single, 0, data.saved_param_single_name, name, mln::db_text_encoding::utf8);
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        co_await mln::response::co_respond(reply_data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], user_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text));

        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_single, target);
}
dpp::task<void> mln::db_delete::user(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const {
    const dpp::command_value& user_param = event_data.get_parameter("user");
    const dpp::snowflake target = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ 0 };

    if (target == 0) {
        co_await mln::response::co_respond(reply_data, "Failed to retrieve user parameter!", true, "Failed to retrieve user parameter!");
        co_return;
    }
    
    if (target != cmd_data.data.usr_id && !mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(reply_data, "Failed to delete the given user records, admin permission required to delete records owned by someone else!", false, {});
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(data.saved_user, 0, data.saved_param_user_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_user, 0, data.saved_param_user_user, static_cast<int64_t>(target));
    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        co_await mln::response::co_respond(reply_data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_user, target);
}
dpp::task<void> mln::db_delete::guild(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const {
    if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(reply_data, "Failed to delete the guild records, admin permission required!", false, {});
        co_return;
    }

    if (cmd_data.data.guild_id != cmd_data.cmd_usr->guild_id) {
        co_await mln::response::co_respond(reply_data, "Failed to delete the guild records, mismatch found between the user guild id and the guild where the command was invoked!", true,
            "Failed to delete the guild records, mismatch found between the user guild id and the guild where the command was invoked!");
        co_return;
    }

    const mln::db_result_t res = db.bind_parameter(data.saved_guild, 0, 1, static_cast<int64_t>(cmd_data.data.guild_id));
    if (res.type != mln::db_result::ok) {
        co_await mln::response::co_respond(reply_data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_guild, 0);
}
dpp::task<void> mln::db_delete::self(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data) const {

    const mln::db_result_t res = db.bind_parameter(data.saved_self, 0, 1, static_cast<int64_t>(cmd_data.data.usr_id));
    if (res.type != mln::db_result::ok) {
        co_await mln::response::co_respond(reply_data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    co_await mln::db_delete::exec(event_data, reply_data, cmd_data, data.saved_self, cmd_data.data.usr_id);
}
struct db_delete_url_data_t {
    std::string url;
    uint64_t user;
};
struct db_delete_lists_data_t {
    std::vector<std::vector<dpp::snowflake>> lists;
};
dpp::task<void> mln::db_delete::exec(const dpp::slashcommand_t& event_data, event_data_lite_t& reply_data, db_cmd_data_t& cmd_data, const size_t stmt, const uint64_t target) const {
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

        co_await mln::response::co_respond(reply_data, err_text, true, std::format("{} Error: [{}], err_text: [{}]",
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

        const auto it3 = visited_guild_by_channel.find({ url_guild, url_channel });
        if (it3 == visited_guild_by_channel.end()) {
            visited_guild_by_channel.insert({ url_guild, url_channel });

            //Retrieve guild data
            const std::optional<std::shared_ptr<const dpp::guild>> guild = co_await mln::caches::get_guild_task(url_guild, reply_data);
            if (!guild.has_value()) {
                co_return;
            }

            //Retrieve channel data
            const std::optional<std::shared_ptr<const dpp::channel>> channel = co_await mln::caches::get_channel_task(url_channel, reply_data, &event_data.command.channel, &event_data.command.resolved.channels);
            if (!channel.has_value()) {
                co_return;
            }

            //Retrieve bot information
            const std::optional<std::shared_ptr<const dpp::guild_member>> bot_opt = co_await mln::caches::get_member_task(url_guild, reply_data.app_id, reply_data, &event_data.command.member, &event_data.command.resolved.members);
            if (!bot_opt.has_value()) {
                co_return;
            }

            const std::optional<dpp::permission> bot_perm = co_await mln::perms::get_computed_permission_task(guild.value()->owner_id, *(channel.value()), *(bot_opt.value()), reply_data, &event_data.command.resolved.roles, &event_data.command.resolved.member_permissions);
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
                            if (mln::utility::conf_callback_is_error(co_await bot().co_message_delete(msg_to_delete, map_pair.first), reply_data, false,
                                std::format("Failed to delete message [{}] from channel [{}]!", static_cast<uint64_t>(msg_to_delete), map_pair.first))) {

                                ++failed_deletes;
                            }
                        }
                    }
                    else {
                        mln::utility::create_event_log_error(cmd_data.data, std::format("Failed to delete bulk messages from channel: [{}]! Error: [{}], details: [{}].", 
                            map_pair.first, mln::get_json_err_text(err.code), err.human_readable));

                        failed_deletes += list.size();
                    }
                }
            }
            else {
                if (mln::utility::conf_callback_is_error(co_await bot().co_message_delete(list[0], map_pair.first), reply_data, false,
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

    co_await mln::response::co_respond(reply_data, final_text, false, {});
    if (error) {
        mln::utility::create_event_log_error(cmd_data.data, final_text);
    }

    co_return;
}

dpp::task<void> mln::db_delete::help(db_cmd_data_t& cmd_data) const {
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

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the db delete help text!");

    co_return;
}
