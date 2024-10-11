#include "commands/base_action.h"
#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"
#include "commands/slash/db/db_help.h"
#include "commands/slash/db/db_init_type_flag.h"
#include "database/db_saved_stmt_state.h"
#include "utility/response.h"

#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>

#include <format>

const dpp::message mln::db_help::s_info = dpp::message{ "Information regarding the `/db` commands..." }
.set_flags(dpp::m_ephemeral)
.add_embed(dpp::embed{}.set_description(R"""(The `/db` commands are designed to interact with the bot's database. The primary function of the database is to store Discord messages and display their contents when requested.
Most of the database commands are related to the local Discord server from which the command is invoked, allowing interaction only with records created on that server. The only exception is the `/db delete self` command.

The main database commands are:

- `/db insert`: A collection of commands that allows the insertion of new records into the database.
- `/db show`: A collection of commands that displays a list of records to the user, often used to find a specific record name to extract later using the `/db select` commands.
- `/db select`: A collection of commands that extracts and displays the content of a specific record from the database.
- `/db update`: A collection of commands that modifies an existing record in the database.
- `/db delete`: A collection of commands that deletes one or more records from the database and also attempts to remove the stored messages related to the deleted records.
- `/db config`: A collection of commands related to database configuration.
- `/db privacy`: A collection of commands related to the bot's privacy policy.

Each of these command sets has a `help` variant, providing in-depth information about the respective commands.)"""));

mln::db_help::db_help(dpp::cluster& cluster) : base_db_command{ cluster } {

    cbot().log(dpp::loglevel::ll_debug, std::format("db_help: [{}].", mln::get_saved_stmt_state_text(is_db_initialized())));
}

dpp::task<void> mln::db_help::command(const dpp::slashcommand_t&, db_cmd_data_t& cmd_data, const db_command_type) {
    co_await mln::response::co_respond(cmd_data.data, mln::db_help::s_info, false, "Failed to reply with the db help text!");
    co_return;
}

mln::db_init_type_flag mln::db_help::get_requested_initialization_type(const db_command_type cmd) const noexcept {
	return db_init_type_flag::none;
}

mln::db_saved_stmt_state mln::db_help::is_db_initialized() const noexcept
{
    return mln::db_saved_stmt_state::initialized;
}
