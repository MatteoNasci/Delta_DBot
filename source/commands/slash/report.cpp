#include "commands/slash/report.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::report::report(mln::bot_delta* const delta) : base_slashcommand(delta,
	std::move(dpp::slashcommand("report_issue", "Report a bug or anything else related to the bot.", delta->bot.me.id)
		.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true)
			.set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
			.set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg())))))), 
	saved_insert_rep_query(), guild_param_index(), user_param_index(), rep_text_param_index(), valid_saved_stmt(true) {

	auto res = delta->db.save_statement("INSERT OR ABORT INTO report (guild_id, user_id, report_text) VALUES(:NNN, :VVV, :TTT);", saved_insert_rep_query); 
	if (res != mln::db_result::ok) {
		delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert report query! " + mln::database_handler::get_name_from_result(res) + ". " + delta->db.get_last_err_msg());
		valid_saved_stmt = false;
	}else {
		auto res1 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":NNN", guild_param_index);
		auto res2 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":VVV", user_param_index);
		auto res3 = delta->db.get_bind_parameter_index(saved_insert_rep_query, 0, ":TTT", rep_text_param_index);
		if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
			delta->bot.log(dpp::loglevel::ll_error, "Failed to save insert report query param indexes! " + mln::database_handler::get_name_from_result(res) + ". " + delta->db.get_last_err_msg());
			valid_saved_stmt = false;
		}
	}
}

dpp::job mln::report::command(dpp::slashcommand_t event){
	if (!valid_saved_stmt) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, missing db query!");
		event.reply(dpp::message("Failed to log the report, internal database error!").set_flags(dpp::m_ephemeral));
		co_return;
	}

	auto thinking = event.co_thinking(true);

	const std::string text = std::get<std::string>(event.get_parameter("description"));
	const int64_t guild_id = event.command.guild_id;
	const int64_t user_id = event.command.usr.id;

	auto res1 = delta()->db.bind_parameter(saved_insert_rep_query, 0, guild_param_index, guild_id);
	auto res2 = delta()->db.bind_parameter(saved_insert_rep_query, 0, user_param_index, user_id);
	auto res3 = delta()->db.bind_parameter(saved_insert_rep_query, 0, rep_text_param_index, text.c_str(), text.length(), mln::db_destructor_behavior::transient_b, mln::db_text_encoding::utf8);

	if (res1 != mln::db_result::ok || res2 != mln::db_result::ok || res3 != mln::db_result::ok) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, failed parameters binding!");
		co_await thinking;
		event.edit_response(dpp::message("Failed to log the report, internal database error!").set_flags(dpp::m_ephemeral));
		co_return;
	}

	auto res4 = delta()->db.exec(saved_insert_rep_query, mln::database_callbacks_t());
	if (res4 != mln::db_result::ok) {
		delta()->bot.log(dpp::loglevel::ll_error, "Cannot perform the report command, failed execution! " + mln::database_handler::get_name_from_result(res4) + ". " + delta()->db.get_last_err_msg());
		co_await thinking;
		event.edit_response(dpp::message("Failed to log the report, internal database error!").set_flags(dpp::m_ephemeral));
		co_return;
	}

	delta()->bot.log(dpp::ll_warning, "[REPORT] " + text);
	co_await thinking;
	event.edit_response(dpp::message("Report received, Thanks!").set_flags(dpp::m_ephemeral));
}