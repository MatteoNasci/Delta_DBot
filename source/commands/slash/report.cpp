#include "commands/slash/report.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::report::report(mln::bot_delta* const delta) : base_slashcommand(delta,
	std::move(dpp::slashcommand("report_issue", "Report a bug or anything else related to the bot.", delta->bot.me.id)
		.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true)
			.set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
			.set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg())))))), 
	saved_insert_rep_query(), guild_param_index(), user_param_index(), rep_text_param_index(), valid_saved_stmt(true) {

	const mln::db_result res = delta->db.save_statement("INSERT OR ABORT INTO report (guild_id, user_id, report_text) VALUES(:NNN, :VVV, :TTT);", saved_insert_rep_query); 
	if (res != mln::db_result::ok) {
		delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert report query! " + mln::database_handler::get_name_from_result(res) + ". " + delta->db.get_last_err_msg());
		valid_saved_stmt = false;
	}else {
		const mln::db_result res1 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":NNN", guild_param_index);
		const mln::db_result res2 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":VVV", user_param_index);
		const mln::db_result res3 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":TTT", rep_text_param_index);
		if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
			delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert report query param indexes! " + mln::database_handler::get_name_from_result(res) + ". " + delta->db.get_last_err_msg());
			valid_saved_stmt = false;
		}
	}
}

dpp::task<void> mln::report::command(const dpp::slashcommand_t& event_data){
	if (!valid_saved_stmt) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, missing db query!");
		event_data.reply(dpp::message("Failed to log the report, internal database error!").set_flags(dpp::m_ephemeral));
		co_return;
	}

	dpp::async<dpp::confirmation_callback_t> thinking = event_data.co_thinking(true);

	const std::string text = std::get<std::string>(event_data.get_parameter("description"));
	const int64_t guild_id = event_data.command.guild_id;
	const int64_t user_id = event_data.command.usr.id;

	const mln::db_result res1 = delta()->db.bind_parameter(saved_insert_rep_query, 0, guild_param_index, guild_id);
	const mln::db_result res2 = delta()->db.bind_parameter(saved_insert_rep_query, 0, user_param_index, user_id);
	const mln::db_result res3 = delta()->db.bind_parameter(saved_insert_rep_query, 0, rep_text_param_index, text.c_str(), text.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);

	if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, failed parameters binding!");
		co_await thinking;
		event_data.edit_response(dpp::message("Failed to log the report, internal database error!"));
		co_return;
	}

	const mln::db_result res4 = delta()->db.exec(saved_insert_rep_query, mln::database_callbacks_t());
	if (res4 != mln::db_result::ok) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, failed execution! " + mln::database_handler::get_name_from_result(res4) + ". " + delta()->db.get_last_err_msg());
		co_await thinking;
		event_data.edit_response(dpp::message("Failed to log the report, internal database error!"));
		co_return;
	}

	delta()->bot.log(dpp::ll_warning, "[REPORT] " + text);
	co_await thinking;
	event_data.edit_response(dpp::message("Report received, Thanks!"));
}