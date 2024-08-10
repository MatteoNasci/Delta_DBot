#include "commands/report.h"
#include "bot_delta.h"

static constexpr dpp::command_option_range s_min_msg_length{1};

dpp::task<void> mln::report::command(const dpp::slashcommand_t& event){
	const std::string text = std::get<std::string>(event.get_parameter("description"));
	mln::bot_delta::delta().bot.log(dpp::ll_warning, "[REPORT] " + text);
	event.reply(dpp::message("Report received, Thanks!").set_flags(dpp::m_ephemeral));
	co_return;
}

dpp::slashcommand mln::report::get_command(){
	return dpp::slashcommand(mln::report::get_command_name(), "Report a bug or anything else related to the bot.", mln::bot_delta::delta().bot.me.id)
				.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true).set_min_length(s_min_msg_length));
}

std::string mln::report::get_command_name(){
	return "report";
}
