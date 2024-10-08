#include "commands/slash/base_slashcommand.h"
#include "commands/slash/help.h"
#include "utility/event_data_lite.h"
#include "utility/response.h"
#include "utility/utility.h"
#include "version.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/coro/job.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/permissions.h>

#include <format>
#include <functional>
#include <optional>
#include <type_traits>

mln::help::help(dpp::cluster& cluster) : base_slashcommand{ cluster,
    std::move(dpp::slashcommand(mln::utility::prefix_dev("help"), "Display information about this bot's commands.", cluster.me.id)
        .set_default_permissions(dpp::permissions::p_use_application_commands)) } {}

dpp::job mln::help::command(dpp::slashcommand_t event_data) const {
    static const dpp::message s_info = dpp::message{ std::format("Information regarding the bot's commands (version: [{}])...", mln::get_version()) }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The bot is primarily focused on its database feature, but it also includes other miscellaneous commands.

The main bot commands are:
- `/db ...`: A collection of commands that allows users to interact with the database. For more information, use the `/db help generic` command.
- `/mog ...`: A collection of commands that allows users to interact with the game *Eminence in Shadow : Master of Garden* utilities. For more information, use the `/mog help generic` command.
- `/info`: A command that displays general information about the bot.
- `/report_issue`: A command used to notify the developer of an issue or bug with the bot, or to suggest improvements or new features.
- `/pm`: A command that sends a direct message to a specific user.
- `/ping`: A command that displays the bot's ping.
- `/avatar`: A command that retrieves the avatar of the target user.
- `/add_role`: A command that assigns a given role to the target user.
- `/add_emoji`: A command that adds a new emoji to the server.
- `/changelog`: A command that displays the changes made to the bot's functionalities over time.)"""));

    event_data_lite_t lite_data{ event_data, bot(), true };
    if (!mln::response::is_event_data_valid(lite_data)) {
        mln::utility::create_event_log_error(lite_data, "Failed help, the event is incorrect!");
        co_return;
    }

    co_await mln::response::co_respond(lite_data, dpp::message{ s_info }, false, "Failed to reply with the bot help text!");
    co_return;
}

std::optional<std::function<void()>> mln::help::job(dpp::slashcommand_t) const
{
    log_incorrect_command();
    return std::nullopt;
}

bool mln::help::use_job() const
{
    return false;
}
