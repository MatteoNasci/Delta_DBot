#include "commands/base_action.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_arma.h"
#include "commands/slash/mog/mog_cmd_data.h"
#include "commands/slash/mog/mog_command_type.h"
#include "commands/slash/mog/mog_init_type_flag.h"
#include "commands/slash/mog/mog_team.h"
#include "commands/slash/mog/mog_team_data.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_column_data.h"
#include "database/db_result.h"
#include "database/db_saved_stmt_state.h"
#include "enum/flags.h"
#include "time/time.h"
#include "utility/constants.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <dpp/timer.h>

#include <array>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

static constexpr size_t s_pagination_timeout = 120;
static constexpr size_t s_pagination_max_text_size = 2000;

static constexpr uint64_t s_seconds_in_minute = 60;
static constexpr uint64_t s_seconds_in_hour = s_seconds_in_minute * 60;
static constexpr uint64_t s_seconds_in_day = s_seconds_in_hour * 24;
static constexpr uint64_t s_seconds_in_2_day = s_seconds_in_day * 2;
static constexpr uint64_t s_seconds_in_4_day = s_seconds_in_day * 4;
static constexpr uint64_t s_seconds_in_28_days = s_seconds_in_day * 28;

mln::mog::mog_arma::mog_arma(dpp::cluster& cluster, mln::mog::mog_team& teams_handler, const std::string& database_file_name) : base_mog_command{ cluster }, 
teams_handler{ teams_handler }, database{}, db_optimize_timer{}, insert_guild_param{}, insert_time_param{}, saved_optimize_db{}, saved_insert_arma_time{}, saved_select_all{}, saved_select{}, guilds_arma_date{}
{
	std::filesystem::create_directory(std::filesystem::current_path().concat("/dbs"));

	mln::db_result_t res = database.open_connection(std::format("dbs/{}.db", database_file_name));
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while connecting to dbs/{}.db database. Error: [{}], details: [{}].", database_file_name, mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	res = database.exec("PRAGMA foreign_keys = ON;PRAGMA optimize=0x10002;", mln::database_callbacks_t());
	if (mln::database_handler::is_exec_error(res.type)) {
		const std::string err_msg = std::format("An error occurred while executing pragmas for dbs/{}.db. Error: [{}], details: [{}].", database_file_name, mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	res = database.exec("CREATE TABLE IF NOT EXISTS arma_time( guild_id INTEGER PRIMARY KEY NOT NULL, start_time INTEGER NOT NULL);", mln::database_callbacks_t());
	if (mln::database_handler::is_exec_error(res.type)) {
		const std::string err_msg = std::format("An error occurred while creating the arma_time table. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	res = database.save_statement("INSERT OR REPLACE INTO arma_time(guild_id, start_time) VALUES(:GGG, :TTT) RETURNING guild_id;", saved_insert_arma_time);
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while saving the insert arma_time stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}
	else {
		const mln::db_result_t res11 = database.get_bind_parameter_index(saved_insert_arma_time, 0, ":GGG", insert_guild_param);
		const mln::db_result_t res12 = database.get_bind_parameter_index(saved_insert_arma_time, 0, ":TTT", insert_time_param);
		if (res11.type != mln::db_result::ok || res12.type != mln::db_result::ok) {
			const std::string err_msg = std::format("Failed to save the insert arma_time stmt param indexes! guild_param: [{}, {}], time_param: [{}, {}].",
				mln::database_handler::get_name_from_result(res11.type), res11.err_text,
				mln::database_handler::get_name_from_result(res12.type), res12.err_text);
			throw std::exception(err_msg.c_str());
		}
	}

	res = database.save_statement("PRAGMA optimize;", saved_optimize_db);
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while saving the optimize db stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	res = database.save_statement("SELECT guild_id, start_time FROM arma_time;", saved_select_all);
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while saving the select all db stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	res = database.save_statement("SELECT start_time FROM arma_time WHERE guild_id = ?;", saved_select);
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while saving the select db stmt. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	static constexpr uint64_t s_optimize_timer_seconds{ 60 * 60 * 24 };
	db_optimize_timer = bot().start_timer([this](dpp::timer timer) {
		cbot().log(dpp::loglevel::ll_debug, "Optimizing arma database...");
		const mln::db_result_t res = database.exec(saved_optimize_db, mln::database_callbacks_t());
		if (mln::database_handler::is_exec_error(res.type)) {
			cbot().log(dpp::loglevel::ll_error, std::format("An error occurred while optimizing arma database. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text));
		}
		else {
			cbot().log(dpp::loglevel::ll_debug, "Arma database optimized!");
		}
		}, s_optimize_timer_seconds);

	res = mln::mog::mog_arma::load_guilds_arma_time();
	if (res.type != mln::db_result::ok) {
		const std::string err_msg = std::format("An error occurred while loading the arma times. Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);
		throw std::exception(err_msg.c_str());
	}

	cbot().log(dpp::loglevel::ll_debug, std::format("mog_arma: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

mln::mog::mog_arma::~mog_arma()
{
	database.close_connection();
}

dpp::task<void> mln::mog::mog_arma::command(const dpp::slashcommand_t& event_data, mln::mog::mog_cmd_data_t& cmd_data, const mln::mog::mog_command_type type)
{
	switch (type) {
	case mln::mog::mog_command_type::cooldown:
		co_await mln::mog::mog_arma::cooldown(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::raw_cooldown:
		co_await mln::mog::mog_arma::raw_cooldown(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::show_cooldowns:
		co_await mln::mog::mog_arma::show_cooldowns(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::start:
		co_await mln::mog::mog_arma::start(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::scheduled:
		co_await mln::mog::mog_arma::scheduled(event_data, cmd_data);
		break;
	case mln::mog::mog_command_type::help:
		co_await mln::mog::mog_arma::help(cmd_data);
		break;
	default:
		co_await mln::response::co_respond(cmd_data.data, "Failed command, the given sub_command is not supported!", true,
			std::format("Failed command, the given sub_command [{}] is not supported for /mog arma!", mln::mog::get_cmd_type_text(type)));
		break;
	}
}

mln::mog::mog_init_type_flag mln::mog::mog_arma::get_requested_initialization_type(const mln::mog::mog_command_type cmd) const noexcept
{
	switch (cmd) {
	case mln::mog::mog_command_type::cooldown:
	case mln::mog::mog_command_type::raw_cooldown:
	case mln::mog::mog_command_type::show_cooldowns:
	case mln::mog::mog_command_type::start:
		return mln::flags::add(mln::mog::mog_init_type_flag::cmd_data, mln::mog::mog_init_type_flag::thinking);
	case mln::mog::mog_command_type::scheduled:
		return mln::mog::mog_init_type_flag::cmd_data;
	case mln::mog::mog_command_type::help:
		return mln::mog::mog_init_type_flag::none;
	default:
		return mln::mog::mog_init_type_flag::all;
	}
}

mln::db_saved_stmt_state mln::mog::mog_arma::is_db_initialized() const noexcept
{
	return db_saved_stmt_state::initialized;
}

dpp::task<void> mln::mog::mog_arma::cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
	static constexpr uint64_t s_total_cd_delay = static_cast<uint64_t>(5 * 60);

	co_await mln::mog::mog_arma::common_cooldown(event_data, cmd_data, s_total_cd_delay);
}

dpp::task<void> mln::mog::mog_arma::start(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data)
{
	if (!mln::perms::check_permissions(cmd_data.cmd_usr_perm, dpp::permissions::p_administrator)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to set arma time! Only admins have access to this command!", false, {});
		co_return;
	}

	const dpp::command_value& day_param = event_data.get_parameter("day");
	const dpp::command_value& month_param = event_data.get_parameter("month");
	const dpp::command_value& year_param = event_data.get_parameter("year");
	const dpp::command_value& hour_param = event_data.get_parameter("utc_hour");

	if (!std::holds_alternative<int64_t>(month_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to set arma time! Failed to retrieve the month value!", true, {});
		co_return;
	}

	const int64_t month = std::get<int64_t>(month_param);
	if (month < 1 || month > 12) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to set arma time! The retrieved month value is outside the valid range of [{}/{}]!",
				1, 12),
			false, {});
		co_return;
	}

	int64_t max_day_value;
	switch (month) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		max_day_value = 31;
		break;
	case 2:
		max_day_value = 28;
		break;
	default:
		max_day_value = 30;
		break;
	}


	if (!std::holds_alternative<int64_t>(day_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to set arma time! Failed to retrieve the day value!", true, {});
		co_return;
	}

	const int64_t day = std::get<int64_t>(day_param);
	if (day < 1 || day > max_day_value) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to set arma time! The retrieved day value is outside the valid range of [{}/{}]!",
				1, max_day_value),
			false, {});
		co_return;
	}


	if (!std::holds_alternative<int64_t>(year_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to set arma time! Failed to retrieve the year value!", true, {});
		co_return;
	}

	const int64_t year = std::get<int64_t>(year_param);
	if (year < static_cast<int64_t>(mln::constants::get_min_arma_year()) || year > static_cast<int64_t>(mln::constants::get_max_arma_year())) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to set arma time! The retrieved year value is outside the valid range of [{}/{}]!",
				mln::constants::get_min_arma_year(), mln::constants::get_max_arma_year()),
			false, {});
		co_return;
	}


	if (!std::holds_alternative<int64_t>(hour_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to set arma time! Failed to retrieve the hour value!", true, {});
		co_return;
	}

	const int64_t hour = std::get<int64_t>(hour_param);
	if (hour < 0 || hour > 23) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to set arma time! The retrieved hour value is outside the valid range of [{}/{}]!",
				0, 23),
			false, {});
		co_return;
	}

	const uint64_t unix_timestamp = mln::time::convert_date_to_UNIX({ static_cast<int>(day), static_cast<int>(month), static_cast<int>(year), static_cast<int>(hour), 0, 0 });

	//Bind query parameters
	const mln::db_result_t res1 = database.bind_parameter(saved_insert_arma_time, 0, insert_guild_param, static_cast<int64_t>(cmd_data.data.guild_id));
	const mln::db_result_t res2 = database.bind_parameter(saved_insert_arma_time, 0, insert_time_param, static_cast<int64_t>(unix_timestamp));

	//Check if any error occurred in the binding process, in case return an error
	if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to bind query parameters, internal error!", true,
			std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], time_param: [{}, {}].",
				mln::database_handler::get_name_from_result(res1.type), res1.err_text,
				mln::database_handler::get_name_from_result(res2.type), res2.err_text));
		co_return;
	}

	//Prepare callbacks for query execution
	bool found = false;
	const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&found);

	//Execute query and return an error if the query failed or if no element was added
	const mln::db_result_t res = database.exec(saved_insert_arma_time, calls);
	if (mln::database_handler::is_exec_error(res.type) || !found) {
		const std::string err_text = !mln::database_handler::is_exec_error(res.type) && !found ?
			"Failed while executing database query! Failed to insert/update new arma time!" :
			"Failed while executing database query! Internal error!";

		co_await mln::response::co_respond(cmd_data.data, err_text, true,
			std::format("{} Error: [{}], details: [{}].",
				err_text,
				mln::database_handler::get_name_from_result(res.type), res.err_text));
		co_return;
	}

	guilds_arma_date[cmd_data.data.guild_id] = arma_time_list{ unix_timestamp, unix_timestamp + s_seconds_in_day, unix_timestamp + s_seconds_in_2_day };

	co_await mln::response::co_respond(cmd_data.data, std::format("Updated arma time using <t:{}:R> as starting day!", unix_timestamp), false, {});

	co_return;
}

dpp::task<void> mln::mog::mog_arma::scheduled(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data)
{
	std::unordered_map<uint64_t, arma_time_list>::iterator it = guilds_arma_date.find(cmd_data.data.guild_id);
	if (it == guilds_arma_date.end()) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to retrieve next arma date! No data found, use the `/mog arma start` command to set a starting time!", false, {});
		co_return;
	}
	
	const uint64_t current_unix_timestamp = mln::time::get_current_UNIX_time();

	bool updated;
	const uint64_t next_arma_unix_timestamp = mln::mog::mog_arma::get_next_arma_time_upd(current_unix_timestamp, it->second, updated);

	const mln::time::date next_arma_date = mln::time::convert_UNIX_to_date(next_arma_unix_timestamp);

	const dpp::command_value& broadcast_param = event_data.get_parameter("broadcast");
	const bool broadcast = std::holds_alternative<bool>(broadcast_param) ? std::get<bool>(broadcast_param) : false;

	dpp::message result{ std::format("The next scheduled arma event is at {}/{}/{} {}:{}:{} UTC, <t:{}:R> local time!", 
		next_arma_date.day < 10 ? std::format("0{}", next_arma_date.day) : std::to_string(next_arma_date.day),
		next_arma_date.month < 10 ? std::format("0{}", next_arma_date.month) : std::to_string(next_arma_date.month),
		next_arma_date.year, 
		next_arma_date.hour < 10 ? std::format("0{}", next_arma_date.hour) : std::to_string(next_arma_date.hour),
		next_arma_date.minute < 10 ? std::format("0{}", next_arma_date.minute) : std::to_string(next_arma_date.minute),
		next_arma_date.second < 10 ? std::format("0{}", next_arma_date.second) : std::to_string(next_arma_date.second),
		next_arma_unix_timestamp) };
	
	if (!broadcast) {
		result.set_flags(dpp::m_ephemeral);
	}

	co_await event_data.co_reply(result);
}

dpp::task<void> mln::mog::mog_arma::raw_cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
	const dpp::command_value& min_param = event_data.get_parameter("minutes");
	const dpp::command_value& sec_param = event_data.get_parameter("seconds");

	if (!std::holds_alternative<int64_t>(min_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! Failed to retrieve the minutes value!", true, {});
		co_return;
	}

	const int64_t mins = std::get<int64_t>(min_param);
	if (mins < mln::constants::get_min_arma_cd_minutes() || mins > mln::constants::get_max_arma_cd_minutes()) {
		co_await mln::response::co_respond(cmd_data.data, 
			std::format("Failed to update cooldown! The retrieved minutes value is outside the valid range of [{}/{}]!", 
			mln::constants::get_min_arma_cd_minutes(), mln::constants::get_max_arma_cd_minutes()), 
			false, {});
		co_return;
	}

	if (!std::holds_alternative<int64_t>(sec_param)) {
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! Failed to retrieve the seconds value!", true, {});
		co_return;
	}

	const int64_t secs = std::get<int64_t>(sec_param);
	if (mins < mln::constants::get_min_arma_cd_seconds() || mins > mln::constants::get_max_arma_cd_seconds()) {
		co_await mln::response::co_respond(cmd_data.data,
			std::format("Failed to update cooldown! The retrieved seconds value is outside the valid range of [{}/{}]!",
				mln::constants::get_min_arma_cd_seconds(), mln::constants::get_max_arma_cd_seconds()),
			false, {});
		co_return;
	}

	const uint64_t total_cd_delay = static_cast<uint64_t>(secs + (mins * 60));

	co_await mln::mog::mog_arma::common_cooldown(event_data, cmd_data, total_cd_delay);
}

dpp::task<void> mln::mog::mog_arma::common_cooldown(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data, const uint64_t cd_delay) const
{
	const dpp::command_value& name_param = event_data.get_parameter("name");

	const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};
	const mln::utility::text_validity_t validity_data{
		.can_be_null = true,
		.log_if_null = false,
		.can_be_empty = true,
		.log_if_empty = false,
		.log_if_out_of_bounds = true,
		.min_size = mln::constants::get_min_team_name_length(),
		.max_size = mln::constants::get_max_team_name_length() };
	if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, validity_data, "team name"))) {

		co_return;
	}

	const size_t teams_with_user = teams_handler.teams_with_user(cmd_data.data.guild_id, cmd_data.data.usr_id);

	switch (teams_with_user) {
	case 0:
		co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of any team!", false, {});
		co_return;
	case 1:
		break;
	default:
		if (!teams_handler.is_user_in_team(cmd_data.data.guild_id, cmd_data.data.usr_id, name)) {
			const std::string err_text = std::format("Failed to update cooldown! The command user is part of [{}] teams but the name parameter [{}] was either not set or the user is not part of the associated team!", teams_with_user, name);
			co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);
			co_return;
		}
		break;
	}

	const std::optional<mln::mog::mog_team_data_t> team_data = name.empty() ? teams_handler.get_team(cmd_data.data.guild_id, cmd_data.data.usr_id) : teams_handler.get_team(cmd_data.data.guild_id, name);
	if (!team_data.has_value()) {
		const std::string err_text = name.empty() ? "Failed to update cooldown! The command user is not part of any team!" : "Failed to update cooldown! The command user is not part of the given team!";
		co_await mln::response::co_respond(cmd_data.data, err_text, true, err_text);
		co_return;
	}

	for (const mln::mog::mog_team_data_t::user_data_t& u_data : team_data.value().users_id_cd) {
		if (u_data.id == cmd_data.data.usr_id) {
			const uint64_t creation_time = static_cast<uint64_t>(event_data.command.id.get_creation_time());

			mln::mog::mog_team_data_t::user_data_t to_set;
			to_set.id = cmd_data.data.usr_id;
			to_set.cd = creation_time + cd_delay;
			to_set.last_update = creation_time;

			if (!teams_handler.set_user_cooldown(cmd_data.data.guild_id, team_data.value().name, to_set)) {
				co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of the given team!", true, "Failed to update cooldown! The command user is not part of the given team! (cooldown)");
				co_return;
			}

			co_await mln::response::co_respond(cmd_data.data, "Cooldown updated!", false, {});
			co_return;
		}
	}

	co_await mln::response::co_respond(cmd_data.data, "Failed to update cooldown! The command user is not part of the given team!", true, "Failed to update cooldown! The command user is no longer part of the given team! (over)");
}

dpp::task<void> mln::mog::mog_arma::show_cooldowns(const dpp::slashcommand_t& event_data, mog_cmd_data_t& cmd_data) const
{
	const dpp::command_value& name_param = event_data.get_parameter("name");

	const std::string name = std::holds_alternative<std::string>(name_param) ? std::get<std::string>(name_param) : std::string{};
	const mln::utility::text_validity_t validity_data{
		.can_be_null = true,
		.log_if_null = false,
		.can_be_empty = true,
		.log_if_empty = false,
		.log_if_out_of_bounds = true,
		.min_size = mln::constants::get_min_team_name_length(),
		.max_size = mln::constants::get_max_team_name_length() };
	if (!(co_await mln::utility::check_text_validity(name, cmd_data.data, validity_data, "team name"))) {

		co_return;
	}
	const bool show_all_teams = name.empty();

	std::vector<mln::mog::mog_team_data_t> data_to_display{};
	if (show_all_teams) {
		teams_handler.get_teams(cmd_data.data.guild_id, data_to_display);
	}
	else {
		const std::optional<mln::mog::mog_team_data_t> team = teams_handler.get_team(cmd_data.data.guild_id, name);
		if (team.has_value()) {
			data_to_display.push_back(team.value());
		}
	}

	if (data_to_display.empty()) {
		static const std::string s_err_text = "Failed to show teams cooldown data, no team associated with the server!";

		co_await mln::response::co_respond(cmd_data.data, show_all_teams ? s_err_text : std::format("Failed to show teams cooldown data, no team named [{}] found associated with the server!", name), false, {});
		co_return;
	}

	std::vector<std::string> records{};
	records.reserve(data_to_display.size());
	size_t total_size = 0;
	const uint64_t current_time = static_cast<uint64_t>(event_data.command.id.get_creation_time());
	const uint64_t min_cd_valid_time = current_time - 40;
	for (const mln::mog::mog_team_data_t& team : data_to_display) {
		records.emplace_back(mln::mog::mog_team_data_t::to_string(team, min_cd_valid_time, current_time));
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

dpp::task<void> mln::mog::mog_arma::help(mog_cmd_data_t& cmd_data) const
{
	static const dpp::message s_info = dpp::message{ "Information regarding the `/mog arma` commands..." }
		.set_flags(dpp::m_ephemeral)
		.add_embed(dpp::embed{}.set_description(R"""(The `/mog arma` set of commands exists to manage cooldowns and other aspects of MOG's Armageddon event. These commands require at least one existing MOG team.

**Types of commands:**

- **/mog arma start**  
  *Parameters:* day[integer, required], month[integer, required], year[integer, required], utc_hour[integer, required].  
  This command requires the input of the date and time (UTC) for the beginning of the first round of Arma, either in the past or future. 
  This information will be saved and will allow the bot to track when future Arma events (and individual Arma rounds) will occur. Once set, this information will automatically update without the need for further user input.
  Use the command `/mog arma scheduled` to find out the date of the next Arma round.
  This command can only be used by admins.

- **/mog arma scheduled**  
  *Parameters:* broadcast[boolean, optional].  
  This command returns the date of the next Arma round. It requires `/mog arma start` to have been set at least once; after that, no further updates are needed by the bot.
  Setting broadcast = True will display the result to everyone in the channel.

- **/mog arma raw_cooldown**  
  *Parameters:* minutes[integer, required], seconds[integer, required], name[text, optional].  
  This command requires specifying the `minutes` (between 0 and 5) and `seconds` (between 0 and 59), along with an optional `name` parameter.
  A new cooldown for the command user will be set (on the specified team) based on the given minutes and seconds. This cooldown can be viewed using the `/mog arma show_cooldowns` command.
  If the user is part of only one MOG team, the optional parameter `name` can be ignored. Otherwise, it MUST be set to a valid team name that includes the user.

- **/mog arma cooldown**  
  *Parameters:* name[text, optional].  
  This command works the same as `/mog arma raw_cooldown`, but it sets the cooldown to a fixed 5 minutes.
  The new cooldown for the command user will be set (on the specified team). This cooldown can be viewed using the `/mog arma show_cooldowns` command.
  If the user is part of only one MOG team, the optional parameter `name` can be ignored. Otherwise, it MUST be set to a valid team name that includes the user.

- **/mog arma show_cooldowns**  
  *Parameters:* name[text, optional].  
  This command takes an optional `name` parameter associated with a team. If not set, the command will display cooldown data for all MOG teams on the server. If set, it will show cooldown data only for the specified team.
  
  WARNING: A message created with this command will not update itself if someone updates their own cooldown, so it's a good idea to refresh this command if you are waiting for someone's updated cooldown.)"""));

	co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the mog arma help text!");
	co_return;
}

mln::db_result_t mln::mog::mog_arma::load_guilds_arma_time()
{
	guilds_arma_date.clear();

	//Prepare callbacks for query execution
	mln::database_callbacks_t calls{};
	uint64_t temp{};
	calls.type_definer_callback = [](void*, int) { return false; };
	calls.data_adder_callback = [this, &temp](void* ptr, int c, mln::db_column_data_t&& d) {
		if (c == 0) {
			temp = static_cast<uint64_t>((std::get<int64_t>(d.data)));
			return;
		}

		const uint64_t unix_time = static_cast<uint64_t>((std::get<int64_t>(d.data)));
		guilds_arma_date[temp] = arma_time_list{ unix_time, unix_time + s_seconds_in_day, unix_time + s_seconds_in_2_day };
		};

	//Execute query and return an error if the query failed or if no element was added
	const mln::db_result_t res = database.exec(saved_select_all, calls);
	if (mln::database_handler::is_exec_error(res.type)) {
		static const std::string s_err_text = "Failed while executing database query! Internal error!";
		return { res.type, std::format("{} {}", s_err_text, res.err_text) };
	}

	//If dates are older than 4 days then update both the map and database
	const uint64_t current_unix_timestamp = mln::time::get_current_UNIX_time();
	for (std::pair<const uint64_t, arma_time_list>& pair : guilds_arma_date) {
		bool updated;
		const uint64_t next_arma = mln::mog::mog_arma::get_next_arma_time_upd(current_unix_timestamp, pair.second, updated);
		if (updated) {

			//Bind query parameters
			const mln::db_result_t res1 = database.bind_parameter(saved_insert_arma_time, 0, insert_guild_param, static_cast<int64_t>(pair.first));
			const mln::db_result_t res2 = database.bind_parameter(saved_insert_arma_time, 0, insert_time_param, static_cast<int64_t>(pair.second[0]));

			//Check if any error occurred in the binding process, in case return an error
			if (res1.type != mln::db_result::ok || res2.type != mln::db_result::ok) {
				return {res.type, std::format("Failed to bind query parameters, internal error! guild_param: [{}, {}], time_param: [{}, {}].",
						mln::database_handler::get_name_from_result(res1.type), res1.err_text,
						mln::database_handler::get_name_from_result(res2.type), res2.err_text) };
			}

			//Prepare callbacks for query execution
			bool found = false;
			const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&found);

			//Execute query and return an error if the query failed or if no element was added
			const mln::db_result_t res = database.exec(saved_insert_arma_time, calls);
			if (mln::database_handler::is_exec_error(res.type) || !found) {
				const std::string err_text = !mln::database_handler::is_exec_error(res.type) && !found ?
					"Failed while executing database query! Failed to insert/update loaded arma time!" :
					"Failed while executing database query! Internal error!";

				return { res.type, std::format("{} Error: [{}], details: [{}].",
						err_text,
						mln::database_handler::get_name_from_result(res.type), res.err_text) };
			}

		}
	}

	return { mln::db_result::ok, {} };
}

uint64_t mln::mog::mog_arma::get_next_arma_time(const uint64_t current_time, const arma_time_list& times)
{
	for (const uint64_t& timestamp : times) {
		//If the last arma time is aat least within 1 hour of now then I use that as the scheduled arma time
		if (current_time < (timestamp + s_seconds_in_hour)) {
			return timestamp;
		}
	}

	return 0;
}

uint64_t mln::mog::mog_arma::get_next_arma_time_upd(const uint64_t current_time, arma_time_list& times, bool& updated)
{
	updated = false;
	const uint64_t result = mln::mog::mog_arma::get_next_arma_time(current_time, times);
	if (result != 0) {
		return result;
	}

	updated = true;

	const uint64_t time_diff = current_time - times[times.size() - 1];
	const uint64_t cycles_gap = (time_diff / s_seconds_in_28_days) + 1;
	for (uint64_t& timestamp : times) {
		timestamp += (s_seconds_in_28_days * cycles_gap);
	}

	return mln::mog::mog_arma::get_next_arma_time(current_time, times);
}
