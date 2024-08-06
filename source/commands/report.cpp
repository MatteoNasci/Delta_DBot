#include "commands/report.h"

static constexpr dpp::command_option_range s_min_msg_length{1};

dpp::task<void> mln::report::command(bot_delta_data_t& data, const dpp::slashcommand_t& event){
	const std::string text = std::get<std::string>(event.get_parameter("description"));
	data.bot.log(dpp::ll_warning, text);
	event.reply(dpp::message("Report received, Thanks!").set_flags(dpp::m_ephemeral));
	co_return;
}

dpp::slashcommand mln::report::get_command(dpp::cluster& bot){
	return dpp::slashcommand(mln::report::get_command_name(), "Report a bug or anything else related to the bot.", bot.me.id)
				.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true).set_min_length(s_min_msg_length));
}

std::string mln::report::get_command_name(){
	return "report";
}
