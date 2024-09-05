#include "commands/slash/help.h"
#include "bot_delta.h"
#include "version.h"

mln::help::help(mln::bot_delta* const delta) : base_slashcommand(delta,
    std::move(dpp::slashcommand("help", "Display information about this bot's commands.", delta->bot.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands))) {}

dpp::task<void> mln::help::command(const dpp::slashcommand_t& event_data){
    static const dpp::message s_info = dpp::message{ "Information regarding the bot's commands (version: " + std::string(mln::get_version()) + ")..."}
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The bot is primarily focused on its database feature, but it also includes other miscellaneous commands.

The main bot commands are:
- `/db ...`: A collection of commands that allows users to interact with the database. For more information, use the `/db help generic` command.
- `/info`: A command that displays general information about the bot.
- `/report_issue`: A command used to notify the developer of an issue or bug with the bot, or to suggest improvements or new features.
- `/pm`: A command that sends a direct message to a specific user.
- `/ping`: A command that displays the bot's ping.
- `/avatar`: A command that retrieves the avatar of the target user.
- `/add_role`: A command that assigns a given role to the target user.
- `/add_emoji`: A command that adds a new emoji to the server.
- `/changelog`: A command that displays the changes made to the bot's functionalities over time.)"""));

    event_data.reply(dpp::message{ s_info });
    co_return;
}