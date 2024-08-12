#include "commands/slash/report.h"
#include "bot_delta.h"
#include "utility/constants.h"

mln::report::report(bot_delta* const delta) : base_slashcommand(delta,
	std::move(dpp::slashcommand("report", "Report a bug or anything else related to the bot.", delta->bot.me.id)
		.add_option(dpp::command_option(dpp::command_option_type::co_string, "description", "Description of the issue", true)
			.set_min_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_min_characters_reply_msg())))
			.set_max_length(dpp::command_option_range(static_cast<int64_t>(mln::constants::get_max_characters_reply_msg())))))) {}

dpp::job mln::report::command(dpp::slashcommand_t event){
	const std::string text = std::get<std::string>(event.get_parameter("description"));
	delta()->bot.log(dpp::ll_warning, "[REPORT] " + text);
	event.reply(dpp::message("Report received, Thanks!").set_flags(dpp::m_ephemeral));
	//TODO add report to a dedicated db
	co_return;
}