#include "commands/slash/report.h"
#include "utility/constants.h"
#include "utility/utility.h"
#include "utility/response.h"
#include "database/database_handler.h"

#include <dpp/cluster.h>

mln::report::report(dpp::cluster& cluster, database_handler& in_db) : base_slashcommand{ cluster,
	std::move(dpp::slashcommand("report_issue", "Report a bug or anything else related to the bot.", cluster.me.id)
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

dpp::task<void> mln::report::command(const dpp::slashcommand_t& event_data) const {
	mln::utility::conf_callback_is_error(co_await event_data.co_thinking(true), bot());
	if (!valid_saved_stmt) {
		mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Failed to log the report, internal database error!"), bot(), &event_data, 
			"Failed to log the report, internal database error!");
		co_return;
	}

	const std::string text = std::get<std::string>(event_data.get_parameter("description"));
	const int64_t guild_id = event_data.command.guild_id;
	const int64_t user_id = event_data.command.usr.id;

	const mln::db_result_t res1 = db.bind_parameter(saved_insert_rep_query, 0, 1, text.c_str(), text.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);

	if (res1.type != mln::db_result::ok) {
		mln::utility::conf_callback_is_error(
			co_await mln::response::make_response(false, event_data, "Failed to bind query parameters, internal error!"), bot(), &event_data,
			std::format("Failed to bind query parameters, internal error! report_param: [{}, {}].",
				mln::database_handler::get_name_from_result(res1.type), res1.err_text));

		co_return;
	}

	bool db_success = false;
	const mln::database_callbacks_t calls = mln::utility::get_any_results_callback(&db_success);

	const mln::db_result_t res = db.exec(saved_insert_rep_query, calls);
	if (mln::database_handler::is_exec_error(res.type) || !db_success) {
		const std::string err_text = (!mln::database_handler::is_exec_error(res.type) || res.type == mln::db_result::constraint_primary_key) && !db_success ?
			"Failed while executing database query! Unknown error!" :
			std::format("Failed while executing database query! Internal error! Error: [{}], details: [{}].", mln::database_handler::get_name_from_result(res.type), res.err_text);

		mln::utility::conf_callback_is_error(
			co_await mln::response::make_response(false, event_data, "Failed while executing database query! Internal error!"), bot(), &event_data, err_text);
		co_return;
	}

	bot().log(dpp::ll_warning, "[REPORT] " + text);

	if (mln::utility::conf_callback_is_error(co_await mln::response::make_response(false, event_data, "Report received, Thanks!"), bot())) {
		mln::utility::create_event_log_error(event_data, bot(), "Failed to reply with the report text!");
	}
}