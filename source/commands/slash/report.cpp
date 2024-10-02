#include "commands/slash/base_slashcommand.h"
#include "commands/slash/report.h"
#include "database/database_callbacks.h"
#include "database/database_handler.h"
#include "database/db_destructor_behavior.h"
#include "database/db_result.h"
#include "database/db_text_encoding.h"
#include "utility/constants.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>
#include <dpp/permissions.h>

#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

mln::report::report(dpp::cluster& cluster, database_handler& in_db) : base_slashcommand{ cluster,
	std::move(dpp::slashcommand(mln::utility::prefix_dev("report_issue"), "Report a bug or anything else related to the bot.", cluster.me.id)
		.set_default_permissions(dpp::permissions::p_use_application_commands)
		.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true)
			.set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
			.set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg()))))) },
	db{ in_db }, saved_insert_rep_query{}, valid_saved_stmt{ true } {

	const mln::db_result_t res = db.save_statement("INSERT OR ABORT INTO report (report_text) VALUES(?1) RETURNING id;", saved_insert_rep_query); 
	if (res.type != mln::db_result::ok) {
		bot().log(dpp::loglevel::ll_error, std::format("Failed to save insert report stmt! Error: [{}], details: [{}].",
			mln::database_handler::get_name_from_result(res.type), res.err_text));
		valid_saved_stmt = false;
	}
}

dpp::task<void> mln::report::command(dpp::slashcommand_t event_data) const {
	mln::event_data_lite_t lite_data{ event_data, bot(), true };

	if (!mln::response::is_event_data_valid(lite_data)) {
		mln::utility::create_event_log_error(lite_data, "Failed to log the report, the event is incorrect!");
		co_return;
	}

	co_await mln::response::co_think(lite_data, true, false, {});

	if (!valid_saved_stmt) {
		co_await mln::response::co_respond(lite_data, "Failed to log the report, internal database error!", true, "Failed to log the report, internal database error!");
		co_return;
	}

	if (!std::holds_alternative<dpp::command_interaction>(event_data.command.data)) {
		co_await mln::response::co_respond(lite_data, "Failed to log the report, discord error!", true, "Failed to log the report, the event does not hold the correct type of data for parameters!");
		co_return;
	}

	const dpp::command_value& report_text_param = event_data.get_parameter("description");
	std::string text = std::holds_alternative<std::string>(report_text_param) ? std::get<std::string>(report_text_param) : std::string{};

	if (text.empty()) {
		co_await mln::response::co_respond(lite_data, "Failed to log the report, either the report text is empty or it was not possible to retrieve it!", true,
			"Failed to log the report, either the report text is empty or it was not possible to retrieve it!");
		co_return;
	}

	if (!mln::utility::is_ascii_printable(text)) {
		co_await mln::response::co_respond(lite_data, "Failed to bind query parameters, given text is composed of invalid characters! Only ASCII printable characters are accepted [32,126]", false, {});
		co_return;
	}

	const mln::db_result_t res1 = db.bind_parameter(saved_insert_rep_query, 0, 1, text.c_str(), text.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);

	if (res1.type != mln::db_result::ok) {
		co_await mln::response::co_respond(lite_data, "Failed to bind query parameters, internal error!", true,
			std::format("Failed to bind query parameters, internal error! report_param: [{}, {}].",
				mln::database_handler::get_name_from_result(res1.type), res1.err_text));

		co_return;
	}

	bool db_success = false;
	const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

	const mln::db_result_t res = db.exec(saved_insert_rep_query, calls);
	if (mln::database_handler::is_exec_error(res.type) || !db_success) {
		std::string err_text = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !db_success ?
			"Failed while executing database query! Unknown error!" :
			std::format("Failed while executing database query! Internal error! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);

		co_await mln::response::co_respond(lite_data, "Failed while executing database query! Internal error!", true, std::move(err_text));

		co_return;
	}

	bot().log(dpp::ll_warning, std::format("[REPORT] {}", text));

	co_await mln::response::co_respond(lite_data, "Report received, Thanks!", false, "Failed to reply with the report text!");
}

std::optional<std::function<void()>> mln::report::job(dpp::slashcommand_t event_data) const
{
	log_incorrect_command();
	return std::nullopt;
}

bool mln::report::use_job() const
{
	return false;
}
