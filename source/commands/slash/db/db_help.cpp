#include "commands/slash/db/db_help.h"
#include "utility/utility.h"

mln::db_help::db_help(bot_delta* const delta) : base_db_command(delta) {
}

dpp::task<void> mln::db_help::command(const dpp::slashcommand_t& event_data, const db_cmd_data_t& cmd_data, db_command_type type, std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking){
    static const dpp::message s_info = dpp::message{ "Information regarding the `/db` commands..." }
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

    event_data.reply(dpp::message{ s_info });
    co_return;
}

mln::db_init_type_flag mln::db_help::get_requested_initialization_type(db_command_type cmd){
	return db_init_type_flag::none;
}
