#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_cmd_data.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "commands/slash/mog/mog_team.h"
#include "commands/slash/mog/mog_team_data.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_column_data.h"
#include "database/db_result.h"
#include "database/db_text_encoding.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

static constexpr size_t s_pagination_timeout = 120;
static constexpr size_t s_pagination_max_text_size = 2000;

mln::mog::mog_team::mog_team(dpp::cluster& cluster, database_handler& db) : base_mog_command{ cluster }, teams_mutex{}, teams_data_cache{},
data{ .valid_stmt = true }, del_data{ .valid_stmt = true }, member_data{ .valid_stmt = true }, 
del_member_data{ .valid_stmt = true }, show_data{ .valid_stmt = true }, show_team_data{ .valid_stmt = true }, show_all_data{ .valid_stmt = true }, db{ db }
{
    const mln::db_result_t res1 = db.save_statement("INSERT OR ABORT INTO mog_team(guild_id, name, channel, role) VALUES(:GGG, :NNN, :CCC, :RRR) RETURNING guild_id;", data.saved_stmt);
    if (res1.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert mog team stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res1.type), res1.err_text));
        data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(data.saved_stmt, 0, ":GGG", data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(data.saved_stmt, 0, ":NNN", data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(data.saved_stmt, 0, ":CCC", data.saved_param_channel);
        const mln::db_result_t res14 = db.get_bind_parameter_index(data.saved_stmt, 0, ":RRR", data.saved_param_role);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok || res14.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert mog team stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], channel_param: [{}, {}], role_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text,
                mln::database_handler::get_name_from_result(res14.type), res14.err_text));
            data.valid_stmt = false;
        }
    }

    const mln::db_result_t res2 = db.save_statement("DELETE FROM mog_team WHERE guild_id = :GGG AND name = :NNN RETURNING guild_id;", del_data.saved_stmt);
    if (res2.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete mog team stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        del_data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(del_data.saved_stmt, 0, ":GGG", del_data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(del_data.saved_stmt, 0, ":NNN", del_data.saved_param_name);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete mog team stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
            del_data.valid_stmt = false;
        }
    }

    const mln::db_result_t res3 = db.save_statement("INSERT OR ABORT INTO mog_team_member(guild_id, name, user_id) VALUES(:GGG, :NNN, :UUU) RETURNING user_id;", member_data.saved_stmt);
    if (res3.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert mog team member stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        member_data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(member_data.saved_stmt, 0, ":GGG", member_data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(member_data.saved_stmt, 0, ":NNN", member_data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(member_data.saved_stmt, 0, ":UUU", member_data.saved_param_user);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert mog team member stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text,
                mln::database_handler::get_name_from_result(res13.type), res13.err_text));
            member_data.valid_stmt = false;
        }
    }

    const mln::db_result_t res4 = db.save_statement("DELETE FROM mog_team_member WHERE guild_id = :GGG AND name = :NNN AND user_id = :UUU RETURNING user_id;", del_member_data.saved_stmt);
    if (res4.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete mog team member stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res4.type), res4.err_text));
        del_member_data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(del_member_data.saved_stmt, 0, ":GGG", del_member_data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(del_member_data.saved_stmt, 0, ":NNN", del_member_data.saved_param_name);
        const mln::db_result_t res13 = db.get_bind_parameter_index(del_member_data.saved_stmt, 0, ":UUU", del_member_data.saved_param_user);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok || res13.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save delete mog team member stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text, 
                mln::database_handler::get_name_from_result(res13.type), res13.err_text));
            del_member_data.valid_stmt = false;
        }
    }

    const mln::db_result_t res5 = db.save_statement("SELECT t.channel, t.role FROM mog_team AS t WHERE t.guild_id = :GGG AND t.name = :NNN; SELECT m.user_id FROM mog_team_member AS m WHERE m.guild_id = :GGG AND m.name = :NNN;", show_team_data.saved_stmt);
    if (res5.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save select mog team stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res5.type), res5.err_text));
        show_team_data.valid_stmt = false;
    }
    else {
        const mln::db_result_t res11 = db.get_bind_parameter_index(show_team_data.saved_stmt, 0, ":GGG", show_team_data.saved_param_guild);
        const mln::db_result_t res12 = db.get_bind_parameter_index(show_team_data.saved_stmt, 0, ":NNN", show_team_data.saved_param_name);
        if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
            bot().log(dpp::loglevel::ll_error, std::format("Failed to save select mog team stmt param indexes! guild_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res11.type), res11.err_text,
                mln::database_handler::get_name_from_result(res12.type), res12.err_text));
            show_team_data.valid_stmt = false;
        }
    }

    const mln::db_result_t res6 = db.save_statement("SELECT t.name, t.channel, t.role, m.user_id FROM mog_team AS t JOIN mog_team_member AS m ON t.guild_id = m.guild_id AND t.name = m.name WHERE t.guild_id = ?1 ORDER BY t.name ASC;", show_data.saved_stmt);
    if (res6.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save select all mog team stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res6.type), res6.err_text));
        show_data.valid_stmt = false;
    }

    const mln::db_result_t res7 = db.save_statement("SELECT * FROM mog_team; SELECT * FROM mog_team_member;", show_all_data.saved_stmt);
    if (res7.type != mln::db_result::ok) {
        bot().log(dpp::loglevel::ll_error, std::format("Failed to save select all mog teams stmt! Error: [{}], details: [{}].",
            mln::database_handler::get_name_from_result(res7.type), res7.err_text));
        show_all_data.valid_stmt = false;
    }

    bot().log(dpp::loglevel::ll_debug, std::format("Team db: [{}].", is_db_initialized()));

    load_teams();
}

mln::mog::mog_team::mog_team(mog_team&& rhs) noexcept : base_mog_command{ rhs.bot() }, teams_mutex{}, teams_data_cache{ std::move(rhs.teams_data_cache) },
data{ std::move(rhs.data) }, del_data{ std::move(rhs.del_data) }, member_data{ std::move(rhs.member_data) },
del_member_data{ std::move(rhs.del_member_data) }, show_data{ std::move(rhs.show_data) }, show_team_data{ std::move(rhs.show_team_data) }, 
show_all_data{ std::move(rhs.show_all_data) }, db{ rhs.db }
{
}

mln::mog::mog_team::~mog_team()
{
    if (show_all_data.valid_stmt) {
        db.delete_statement(show_all_data.saved_stmt);
    }
    if (show_data.valid_stmt) {
        db.delete_statement(show_data.saved_stmt);
    }
    if (show_team_data.valid_stmt) {
        db.delete_statement(show_team_data.saved_stmt);
    }
    if (del_member_data.valid_stmt) {
        db.delete_statement(del_member_data.saved_stmt);
    }
    if (member_data.valid_stmt) {
        db.delete_statement(member_data.saved_stmt);
    }
    if (del_data.valid_stmt) {
        db.delete_statement(del_data.saved_stmt);
    }
    if (data.valid_stmt) {
        db.delete_statement(data.saved_stmt);
    }
}

dpp::task<void> mln::mog::mog_team::command(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const mog_command_type type) const
{
    switch (type) {
    case mln::mog::mog_command_type::create:
        co_await mln::mog::mog_team::create(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::del:
        co_await mln::mog::mog_team::del(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::join:
        co_await mln::mog::mog_team::join(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::leave:
        co_await mln::mog::mog_team::leave(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::leave_and_join:
        co_await mln::mog::mog_team::leave_and_join(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::show:
        co_await mln::mog::mog_team::show(event_data, cmd_data);
        break;
    case mln::mog::mog_command_type::help:
        co_await mln::mog::mog_team::help(cmd_data);
        break;
    default:
        co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
            std::format("Failed command, the given sub_command [{}] is not supported for /mog team!", mln::mog::get_cmd_type_text(type)));
        break;
    }
}

mln::mog::mog_init_type_flag mln::mog::mog_team::get_requested_initialization_type(const mog_command_type cmd) const
{
    switch (cmd) {
    case mln::mog::mog_command_type::create:
    case mln::mog::mog_command_type::del:
    case mln::mog::mog_command_type::join:
    case mln::mog::mog_command_type::leave:
    case mln::mog::mog_command_type::leave_and_join:
    case mln::mog::mog_command_type::show:
        return mln::mog::mog_init_type_flag::cmd_data | mln::mog::mog_init_type_flag::thinking;
    case mln::mog::mog_command_type::help:
        return mln::mog::mog_init_type_flag::none;
    default:
        return mln::mog::mog_init_type_flag::all;
    }
}

bool mln::mog::mog_team::is_db_initialized() const
{
    return data.valid_stmt && del_data.valid_stmt && member_data.valid_stmt && del_member_data.valid_stmt && show_data.valid_stmt && show_team_data.valid_stmt && show_all_data.valid_stmt;
}

dpp::task<void> mln::mog::mog_team::create(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, this command required admin perms to be used!", false, {});
        co_return;
    }
    
    const dpp::command_value& name_param = event_data.get_parameter("name");
    const dpp::command_value& channel_param = event_data.get_parameter("channel");
    const dpp::command_value& role_param = event_data.get_parameter("role");

    const std::optional<std::string> name = co_await mln::utility::check_text_validity(name_param, cmd_data.data, false, 
        mln::constants::get_min_team_name_length(), mln::constants::get_max_team_name_length(), "team name");
    if (!name.has_value()) {
        co_return;
    }

    const dpp::snowflake channel = std::holds_alternative<dpp::snowflake>(channel_param) ? std::get<dpp::snowflake>(channel_param) : dpp::snowflake{ 0 };
    const dpp::snowflake role = std::holds_alternative<dpp::snowflake>(role_param) ? std::get<dpp::snowflake>(role_param) : dpp::snowflake{ 0 };

    const mln::db_result_t res1 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_name, name.value(), mln::db_text_encoding::utf8);
    const mln::db_result_t res3 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_channel, static_cast<int64_t>(channel));
    const mln::db_result_t res4 = db.bind_parameter(data.saved_stmt, 0, data.saved_param_role, static_cast<int64_t>(role));

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok || res4.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], name_param: [{}, {}], channel_param: [{}, {}], role_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text, 
                mln::database_handler::get_name_from_result(res3.type), res3.err_text, 
                mln::database_handler::get_name_from_result(res4.type), res4.err_text));
        co_return;
    }

    bool result = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&result);

    const mln::db_result_t res = db.exec(data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !result) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !result;
        const std::string err_text = is_user_error ?
            "Failed while executing database query! A team already exists with the same name!" :
            "Failed while executing database query! Internal database error!";

        co_await mln::response::co_respond(cmd_data.data, err_text, true, std::format("{} Error: [{}], err_text: [{}]",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    set_team(mln::mog::mog_team_data_t{ name.value(), cmd_data.data.guild_id, channel, role });

    co_await mln::response::co_respond(cmd_data.data, "Team created!", false, {});
}

dpp::task<void> mln::mog::mog_team::del(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, this command required admin perms to be used!", false, {});
        co_return;
    }

    const dpp::command_value& name_param = event_data.get_parameter("name");

    const std::optional<std::string> name = co_await mln::utility::check_text_validity(name_param, cmd_data.data, false,
        mln::constants::get_min_team_name_length(), mln::constants::get_max_team_name_length(), "team name");
    if (!name.has_value()) {
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(del_data.saved_stmt, 0, del_data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(del_data.saved_stmt, 0, del_data.saved_param_name, name.value(), mln::db_text_encoding::utf8);

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], name_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text));
        co_return;
    }

    bool result = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&result);

    const mln::db_result_t res = db.exec(del_data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !result) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !result;
        const std::string err_text = is_user_error ?
            "Failed while executing database query! Failed to find a team with the given name!" :
            "Failed while executing database query! Internal database error!";

        co_await mln::response::co_respond(cmd_data.data, err_text, true, std::format("{} Error: [{}], err_text: [{}]",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    delete_team(cmd_data.data.guild_id, name.value());

    co_await mln::response::co_respond(cmd_data.data, "Team deleted!", false, {});
}

dpp::task<void> mln::mog::mog_team::join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    const dpp::command_value& name_param = event_data.get_parameter("name");
    const dpp::command_value& user_param = event_data.get_parameter("user");

    const dpp::snowflake user = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ cmd_data.data.usr_id };

    if (user != cmd_data.data.usr_id) {
        if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
            co_await mln::response::co_respond(cmd_data.data, "Error, forcing other members to join a team requires admin perms!", false, {});
            co_return;
        }
    }

    co_await mln::mog::mog_team::join(event_data, cmd_data, user, std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{});
}

dpp::task<void> mln::mog::mog_team::join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const uint64_t target, const std::string& name) const
{
    if (target == 0) {
        co_await mln::response::co_respond(cmd_data.data, "Error, invalid target user!", true, "Error, invalid target user!");
        co_return;
    }
    if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, false,
        mln::constants::get_min_team_name_length(), mln::constants::get_max_team_name_length(), "team name"))) {
        co_return;
    }

    if (!is_team_present(cmd_data.data.guild_id, name)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, the given name is not associated with a team!", false, {});
        co_return;
    }

    if (is_user_in_team(cmd_data.data.guild_id, target, name)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, the user is already part of the given team!", false, {});
        co_return;
    }

    const mln::db_result_t res1 = db.bind_parameter(member_data.saved_stmt, 0, member_data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(member_data.saved_stmt, 0, member_data.saved_param_name, name, mln::db_text_encoding::utf8);
    const mln::db_result_t res3 = db.bind_parameter(member_data.saved_stmt, 0, member_data.saved_param_user, static_cast<int64_t>(target));

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        co_return;
    }

    bool result = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&result);

    const mln::db_result_t res = db.exec(member_data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !result) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !result;
        const std::string err_text = is_user_error ?
            "Failed while executing database query! The given user is already present in the given team!" :
            "Failed while executing database query! Internal database error!";

        co_await mln::response::co_respond(cmd_data.data, err_text, true, std::format("{} Error: [{}], err_text: [{}]",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return;
    }

    add_user_to_team(cmd_data.data.guild_id, target, name);

    co_await mln::response::co_respond(cmd_data.data, "User added to team!", false, {});
}

dpp::task<bool> mln::mog::mog_team::leave(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    const dpp::command_value& name_param = event_data.get_parameter("name");
    const dpp::command_value& user_param = event_data.get_parameter("user");

    const dpp::snowflake user = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ cmd_data.data.usr_id };

    if (user != cmd_data.data.usr_id) {
        if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
            co_await mln::response::co_respond(cmd_data.data, "Error, forcing other members to leave a team requires admin perms!", false, {});
            co_return false;
        }
    }

    co_return co_await mln::mog::mog_team::leave(event_data, cmd_data, user, std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{});
}

dpp::task<bool> mln::mog::mog_team::leave(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const uint64_t target, const std::string& name) const
{
    if (target == 0) {
        co_await mln::response::co_respond(cmd_data.data, "Error, invalid target user!", true, "Error, invalid target user!");
        co_return false;
    }
    if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, false,
        mln::constants::get_min_team_name_length(), mln::constants::get_max_team_name_length(), "team name"))) {
        co_return false;
    }

    if (!is_team_present(cmd_data.data.guild_id, name)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, the given name is not associated with a team!", false, {});
        co_return false;
    }

    if (!is_user_in_team(cmd_data.data.guild_id, target, name)) {
        co_await mln::response::co_respond(cmd_data.data, "Error, the user is not part of the given team!", false, {});
        co_return false;
    }

    const mln::db_result_t res1 = db.bind_parameter(del_member_data.saved_stmt, 0, del_member_data.saved_param_guild, static_cast<int64_t>(cmd_data.data.guild_id));
    const mln::db_result_t res2 = db.bind_parameter(del_member_data.saved_stmt, 0, del_member_data.saved_param_name, name, mln::db_text_encoding::utf8);
    const mln::db_result_t res3 = db.bind_parameter(del_member_data.saved_stmt, 0, del_member_data.saved_param_user, static_cast<int64_t>(target));

    if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok || res3.type != mln::db_result::ok) {
        co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal database error!", true,
            std::format("Failed to bind query parameters, internal database error! guild_param: [{}, {}], name_param: [{}, {}], user_param: [{}, {}].",
                mln::database_handler::get_name_from_result(res1.type), res1.err_text,
                mln::database_handler::get_name_from_result(res2.type), res2.err_text,
                mln::database_handler::get_name_from_result(res3.type), res3.err_text));
        co_return false;
    }

    bool result = false;
    mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&result);

    const mln::db_result_t res = db.exec(del_member_data.saved_stmt, calls);
    if (mln::database_handler::is_exec_error(res.type) || !result) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !result;
        const std::string err_text = is_user_error ?
            "Failed while executing database query! The given user is not present in the given team!" :
            "Failed while executing database query! Internal database error!";

        co_await mln::response::co_respond(cmd_data.data, err_text, true, std::format("{} Error: [{}], err_text: [{}]",
            err_text,
            mln::database_handler::get_name_from_result(res.type), res.err_text));

        co_return false;
    }

    remove_user_from_team(cmd_data.data.guild_id, target, name);

    co_await mln::response::co_respond(cmd_data.data, "User removed from team!", false, {});
    co_return true;
}

dpp::task<void> mln::mog::mog_team::leave_and_join(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    const dpp::command_value& team_to_join_param = event_data.get_parameter("team_to_join");
    const std::string team_to_join = std::holds_alternative<std::string>(team_to_join_param) ? std::get<std::string>(team_to_join_param) : std::string{};
    const dpp::command_value& team_to_leave_param = event_data.get_parameter("team_to_leave");
    const std::string team_to_leave = std::holds_alternative<std::string>(team_to_leave_param) ? std::get<std::string>(team_to_leave_param) : std::string{};

    const dpp::command_value& user_param = event_data.get_parameter("user");

    const dpp::snowflake target = std::holds_alternative<dpp::snowflake>(user_param) ? std::get<dpp::snowflake>(user_param) : dpp::snowflake{ cmd_data.data.usr_id };

    if (target != cmd_data.data.usr_id) {
        if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
            co_await mln::response::co_respond(cmd_data.data, "Error, forcing other members to leave/join a team requires admin perms!", false, {});
            co_return;
        }
    }

    if (!is_user_in_any_team(cmd_data.data.guild_id, target)) {
        co_await mln::mog::mog_team::join(event_data, cmd_data, target, team_to_join);
        co_return;
    }

    if (team_to_join == team_to_leave) {
        co_await mln::response::co_respond(cmd_data.data, "Error, cannot leave and join the same team!", false, {});
        co_return;
    }

    const bool left = co_await mln::mog::mog_team::leave(event_data, cmd_data, target, team_to_leave);
    if (left) {
        co_await mln::mog::mog_team::join(event_data, cmd_data, target, team_to_join);
    }
}

dpp::task<void> mln::mog::mog_team::show(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
    const dpp::command_value& name_param = event_data.get_parameter("name");

    const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};
    if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, true,
        mln::constants::get_min_team_name_length(), mln::constants::get_max_team_name_length(), "team name"))) {

        co_return;
    }
    const bool show_all_teams = name.empty();

    std::vector<mln::mog::mog_team_data_t> data_to_display{};
    if (show_all_teams) {
        get_teams(cmd_data.data.guild_id, data_to_display);
    }
    else {
        const std::optional<mln::mog::mog_team_data_t> team = get_team(cmd_data.data.guild_id, name);
        if (team.has_value()) {
            data_to_display.push_back(team.value());
        }
    }

    if (data_to_display.empty()) {
        static const std::string s_err_text = "Failed to show teams data, no team associated with the server!";

        co_await mln::response::co_respond(cmd_data.data, show_all_teams ? s_err_text : std::format("Failed to show teams data, no team named [{}] found associated with the server!", name), false, {});
        co_return;
    }

    std::vector<std::string> records{};
    records.reserve(data_to_display.size());
    size_t total_size = 0;
    for (const mln::mog::mog_team_data_t& team : data_to_display) {
        records.emplace_back(mln::mog::mog_team_data_t::to_string_no_cd(team));
        total_size += records[records.size() - 1].size();
    }

    if (total_size > mln::constants::get_max_characters_reply_msg()) {
        co_await mln::response::co_respond(cmd_data.data, "Proceeding with pagination...", false, {});

        mln::utility::manage_paginated_embed(mln::utility::paginated_data_t{
            .event_data = cmd_data.data,
            .time_limit_seconds = s_pagination_timeout,
            .text_limit = s_pagination_max_text_size }, std::make_shared<const std::vector<std::string>>(std::move(records)));
    }
    else {
        std::string msg{};
        for (size_t i = 0; i < records.size(); ++i) {
            msg = std::format("{}{}", msg, std::move(records[i]));
        }

        co_await mln::response::co_respond(cmd_data.data, msg, false, {});
    }
}

dpp::task<void> mln::mog::mog_team::help(mog_cmd_data_t& cmd_data) const
{
    static const dpp::message s_info = dpp::message{ "Information regarding the `/mog team` commands..." }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/mog team` set of commands is used to create/delete teams, join/leave teams, and display teams' data.

**Types of commands:**

- **/mog team create**  
  *Parameters:* name[text, required], channel[channel ID, optional], role[role ID, optional].
  This command allows you to create a new MOG team identified by the given `name`, with optional additional data such as a dedicated `channel` and `role`. Each team is uniquely identified by its name; no two teams can share the same name.
  The newly created team will initially be empty and can be filled using the `/mog team join` or `/mog team leave_and_join` commands. Team data will be stored in the bot's database for persistence.
  This command is restricted to admins only.

- **/mog team delete**  
  *Parameters:* name[text, required].
  This command allows you to delete a MOG team identified by the given `name`.
  It will also remove all users associated with the team (equivalent to calling `/mog team leave [name]` for all team members) and clear the database of all data related to the deleted team.
  This command is restricted to admins only.

- **/mog team join**  
  *Parameters:* name[text, required], user[user ID, optional].
  This command allows a user to join the team associated with the given `name`. A user can be part of one or more teams simultaneously but cannot be part of the same team multiple times.
  The optional `user` parameter is used when an admin wants to make a specific user join a team. If not set, the `user` is the command user by default.
  Only admins can make other users join a team, making them the only ones who can effectively use the `user` optional parameter.

- **/mog team leave**  
  *Parameters:* name[text, required], user[user ID, optional].
  This command allows a user to leave the team associated with the given `name`. A user can be part of one or more teams simultaneously but cannot be part of the same team multiple times.
  The optional `user` parameter is used when an admin wants to make a specific user leave a team. If not set, the `user` is the command user by default.
  Only admins can make other users leave a team, making them the only ones who can effectively use the `user` optional parameter.

- **/mog team leave_and_join**  
  *Parameters:* team_to_join[text, required], team_to_leave[text, optional], user[user ID, optional].
  This command allows a user to join the team associated with the `team_to_join` name while leaving another team. A user can be part of one or more teams simultaneously but cannot be part of the same team multiple times.
  The optional `team_to_leave` parameter is used to identify which team to leave, if any. This parameter can be ignored if the user belongs to 0 or 1 teams. However, if the user belongs to 2 or more teams, it MUST be set to a valid team that the user is part of.
  The optional `user` parameter is used when an admin wants to make a specific user leave and join a team. If not set, the `user` is the command user by default.
  Only admins can make other users leave and join teams, making them the only ones who can effectively use the optional `user` parameter.

- **/mog team show**  
  *Parameters:* name[text, optional].
  This command allows the display of information regarding one or more MOG teams. The optional parameter `name` allows the user to either:
  1) Display data about a single, specific team associated with the given name (if the optional parameter is set);
  2) Display data about all teams on the server (if the optional parameter is not set).)"""));

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the mog team help text!");
    co_return;
}


struct members_hasher {
    std::size_t operator()(const std::tuple<uint64_t, std::string>& value) const noexcept {
        const size_t hash1 = std::hash<uint64_t>()(std::get<0>(value));
        const size_t hash2 = std::hash<std::string>()(std::get<1>(value));

        return hash1 ^ (hash2 << 1);
    }
};
struct members_eq {
    bool operator()(const std::tuple<uint64_t, std::string>& lhs, const std::tuple<uint64_t, std::string>& rhs) const noexcept {
        return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs);
    }
};
void mln::mog::mog_team::load_teams() const
{
    std::vector<std::tuple<uint64_t, std::string, uint64_t, uint64_t>> mog_teams{};
    std::unordered_map<std::tuple<uint64_t, std::string>, std::vector<uint64_t>, members_hasher, members_eq> mog_members{};
    std::tuple<uint64_t, std::string, uint64_t> temp_data{};
    size_t current_stmt = 0;
    mln::database_callbacks_t calls;
    calls.callback_data = &current_stmt;
    calls.statement_index_callback = [](void* c, size_t stmt) {*(static_cast<size_t*>(c)) = stmt; };
    calls.type_definer_callback = [](void*, int c) -> bool { return c == 1; };
    calls.data_adder_callback = [&mog_teams, &mog_members, &temp_data](void* stmt_ptr, int c, mln::db_column_data_t&& d) {
            if(*static_cast<size_t*>(stmt_ptr) == 0){
                switch (c) {
                case 0:
                    std::get<0>(temp_data) = static_cast<uint64_t>(std::get<int64_t>(d.data));
                    return;
                case 1:
                    std::get<1>(temp_data) = reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data));
                    return;
                case 2:
                    std::get<2>(temp_data) = static_cast<uint64_t>(std::get<int64_t>(d.data));
                    return;
                case 3:
                    mog_teams.emplace_back(std::get<0>(temp_data), std::get<1>(temp_data), std::get<2>(temp_data), static_cast<uint64_t>(std::get<int64_t>(d.data)));
                    return;
                }
            }
            else {
                switch (c) {
                case 0:
                    std::get<0>(temp_data) = static_cast<uint64_t>(std::get<int64_t>(d.data));
                    return;
                case 1:
                    std::get<1>(temp_data) = reinterpret_cast<const char*>(std::get<const unsigned char*>(d.data));
                    return;
                case 2:
                    mog_members[{ std::get<0>(temp_data), std::get<1>(temp_data) }].push_back(static_cast<uint64_t>(std::get<int64_t>(d.data)));
                    return;
                }
            }
        };
    

    bot().log(dpp::loglevel::ll_debug, "Recovering teams from database...");
    const mln::db_result_t res = db.exec(show_all_data.saved_stmt, calls);
    const bool empty_result = mog_teams.empty();
    if (mln::database_handler::is_exec_error(res.type) || empty_result) {
        const bool is_user_error = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && empty_result;
        const std::string err_text = is_user_error ?
            "Failed while executing database query for loading mog teams! Failed to find any teams!" :
            "Failed while executing database query for loading mog teams! Internal database error!";

        bot().log(dpp::loglevel::ll_error, std::format("{} Error: [{}], details: [{}].", err_text, mln::database_handler::get_name_from_result(res.type), res.err_text));
    }

    bot().log(dpp::loglevel::ll_debug, "Clearing teams cache...");
    clear_teams();
    bot().log(dpp::loglevel::ll_debug, std::format("Cleared cache! Using [{}] recovered teams from db (total users: [{}]) to fill the cache...", mog_teams.size(), mog_members.size()));

    //NOTE: Slow but who cares
    size_t i = 0;
    for (const std::tuple<uint64_t, std::string, uint64_t, uint64_t>& raw_team_data : mog_teams) {
        mln::mog::mog_team_data_t team_data;
        team_data.guild_id = std::get<0>(raw_team_data);
        team_data.name = std::get<1>(raw_team_data);
        team_data.channel_id = std::get<2>(raw_team_data);
        team_data.role_id = std::get<3>(raw_team_data);
        team_data.users_id_cd = std::vector<mln::mog::mog_team_data_t::user_data_t>{};

        const std::unordered_map<std::tuple<uint64_t, std::string>, std::vector<uint64_t>>::const_iterator it = mog_members.find({ std::get<0>(raw_team_data), std::get<1>(raw_team_data) });
        if (it != mog_members.end()) {

            team_data.users_id_cd.reserve(it->second.size());
            for (const uint64_t& id : it->second) {
                team_data.users_id_cd.emplace_back(id, 0, 0);
            }
        }

        set_team(team_data);
    }

    bot().log(dpp::loglevel::ll_debug, std::format("Loaded [{}] guilds with teams from db!", teams_count()));
}

std::optional<mln::mog::mog_team_data_t> mln::mog::mog_team::get_team(const uint64_t guild_id, const std::string& team_name) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return std::nullopt;
    }

    const std::unordered_map<std::string, mln::mog::mog_team_data_t>::const_iterator it = it_map->second.find(team_name);
    if (it == it_map->second.end()) {
        return std::nullopt;
    }
    
    return it->second;
}

std::optional<mln::mog::mog_team_data_t> mln::mog::mog_team::get_team(const uint64_t guild_id, const uint64_t user_id) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return std::nullopt;
    }

    for (const std::pair<const std::string, mln::mog::mog_team_data_t>& pair : it_map->second) {
        for (const mln::mog::mog_team_data_t::user_data_t& u_data : pair.second.users_id_cd) {
            if (u_data.id == user_id) {
                return pair.second;
            }
        }
    }

    return std::nullopt;
}

void mln::mog::mog_team::get_teams(const uint64_t guild_id, std::vector<mln::mog::mog_team_data_t>& out_teams) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };
  
    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return;
    }

    out_teams.reserve(it_map->second.size() + out_teams.size());
    for (const std::pair<std::string, mln::mog::mog_team_data_t>& pair : it_map->second) {
        out_teams.push_back(pair.second);
    }
}

bool mln::mog::mog_team::is_any_team_in_guild(const uint64_t guild_id) const
{
    {
        std::shared_lock<std::shared_mutex> lock{ teams_mutex };

        const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
        if (it_map == teams_data_cache.end()) {
            return false;
        }
    }

    return true;
}

bool mln::mog::mog_team::is_team_present(const uint64_t guild_id, const std::string& team_name) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return false;
    }

    const std::unordered_map<std::string, mln::mog::mog_team_data_t>::const_iterator it = it_map->second.find(team_name);
    return it != it_map->second.end();
}

bool mln::mog::mog_team::is_user_in_any_team(const uint64_t guild_id, const uint64_t user_id) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return false;
    }

    for (const std::pair<const std::string, mln::mog::mog_team_data_t>& pair : it_map->second) {
        for (const mln::mog::mog_team_data_t::user_data_t& u_data : pair.second.users_id_cd) {
            if (u_data.id == user_id) {
                return true;
            }
        }
    }

    return false;
}

size_t mln::mog::mog_team::teams_with_user(const uint64_t guild_id, const uint64_t user_id) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    size_t counter = 0;

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return counter;
    }

    for (const std::pair<const std::string, mln::mog::mog_team_data_t>& pair : it_map->second) {
        for (const mln::mog::mog_team_data_t::user_data_t& u_data : pair.second.users_id_cd) {
            if (u_data.id == user_id) {
                ++counter;
            }
        }
    }

    return counter;
}

bool mln::mog::mog_team::is_user_in_team(const uint64_t guild_id, const uint64_t user_id, const std::string& team_name) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return false;
    }

    const std::unordered_map<std::string, mln::mog::mog_team_data_t>::const_iterator it = it_map->second.find(team_name);
    if (it == it_map->second.end()) {
        return false;
    }

    for (const mln::mog::mog_team_data_t::user_data_t& u_data : it->second.users_id_cd) {
        if (u_data.id == user_id) {
            return true;
        }
    }

    return false;
}

size_t mln::mog::mog_team::teams_count() const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    return teams_data_cache.size();
}

size_t mln::mog::mog_team::guild_teams_count(const uint64_t guild_id) const
{
    std::shared_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::const_iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return 0;
    }

    return it_map->second.size();
}

bool mln::mog::mog_team::is_team_valid(const mln::mog::mog_team_data_t& team) noexcept
{
    return mln::mog::mog_team::is_team_valid(team.guild_id,team.name);
}

bool mln::mog::mog_team::is_team_valid(const uint64_t guild_id, const std::string& team_name) noexcept
{
    return guild_id && !team_name.empty();
}

bool mln::mog::mog_team::set_user_cooldown(const uint64_t guild_id, const std::string& team_name, const mln::mog::mog_team_data_t::user_data_t& user_data) const
{
    if (!mln::mog::mog_team::is_team_valid(guild_id, team_name) || user_data.id == 0) {
        return false;
    }

    {
        std::unique_lock<std::shared_mutex> lock{ teams_mutex };

        const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::iterator it_map = teams_data_cache.find(guild_id);
        if (it_map == teams_data_cache.end()) {
            return false;
        }

        const std::unordered_map<std::string, mln::mog::mog_team_data_t>::iterator it = it_map->second.find(team_name);
        if (it == it_map->second.end()) {
            return false;
        }

        for (mln::mog::mog_team_data_t::user_data_t& u_data : it->second.users_id_cd) {
            if (u_data.id == user_data.id) {
                u_data = user_data;
                return true;
            }
        }
    }

    return false;
}

void mln::mog::mog_team::delete_team(const uint64_t guild_id, const std::string& team_name) const
{
    if (guild_id == 0 || team_name.empty()) {
        return;
    }

    {
        std::unique_lock<std::shared_mutex> lock{ teams_mutex };

        const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::iterator it_map = teams_data_cache.find(guild_id);
        if (it_map == teams_data_cache.end()) {
            return;
        }

        const std::unordered_map<std::string, mln::mog::mog_team_data_t>::iterator it = it_map->second.find(team_name);
        if (it == it_map->second.end()) {
            return;
        }

        it_map->second.erase(it);
    }
}

void mln::mog::mog_team::delete_teams(const uint64_t guild_id) const
{
    if (guild_id == 0) {
        return;
    }

    {
        std::unique_lock<std::shared_mutex> lock{ teams_mutex };

        const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::iterator it_map = teams_data_cache.find(guild_id);
        if (it_map == teams_data_cache.end()) {
            return;
        }

        teams_data_cache.erase(it_map);
    }
}
bool mln::mog::mog_team::add_user_to_team(const uint64_t guild_id, const uint64_t user_id, const std::string& team_name) const
{
    std::unique_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return false;
    }

    const std::unordered_map<std::string, mln::mog::mog_team_data_t>::iterator it = it_map->second.find(team_name);
    if (it == it_map->second.end()) {
        return false;
    }

    for (const mln::mog::mog_team_data_t::user_data_t& u_data : it->second.users_id_cd) {
        if (u_data.id == user_id) {
            return false;
        }
    }

    it->second.users_id_cd.emplace_back(user_id, 0, 0);
    return true;
}

bool mln::mog::mog_team::remove_user_from_team(const uint64_t guild_id, const uint64_t user_id, const std::string& team_name) const
{
    std::unique_lock<std::shared_mutex> lock{ teams_mutex };

    const std::unordered_map<uint64_t, std::unordered_map<std::string, mln::mog::mog_team_data_t>>::iterator it_map = teams_data_cache.find(guild_id);
    if (it_map == teams_data_cache.end()) {
        return false;
    }

    const std::unordered_map<std::string, mln::mog::mog_team_data_t>::iterator it = it_map->second.find(team_name);
    if (it == it_map->second.end()) {
        return false;
    }

    for (std::vector<mln::mog::mog_team_data_t::user_data_t>::iterator it_to_delete = it->second.users_id_cd.begin(); it_to_delete != it->second.users_id_cd.end(); ++it_to_delete) {
        if (it_to_delete->id == user_id) {
            it->second.users_id_cd.erase(it_to_delete);
            return true;
        }
    }

    return false;
}

bool mln::mog::mog_team::set_team(mln::mog::mog_team_data_t team) const
{
    if (!mln::mog::mog_team::is_team_valid(team)) {
        return false;
    }

    {
        std::unique_lock<std::shared_mutex> lock{ teams_mutex };

        teams_data_cache[team.guild_id][team.name] = std::move(team);
    }

    return true;
}

void mln::mog::mog_team::clear_teams() const
{
    if (teams_count() == 0) {
        return;
    }

    {
        std::unique_lock<std::shared_mutex> lock{ teams_mutex };
        teams_data_cache.clear();
    }
}
