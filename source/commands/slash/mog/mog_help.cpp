#include "commands/slash/mog/mog_help.h"
#include <commands/base_action.h>
#include <commands/slash/mog/base_mog_command.h>
#include "utility/response.h"
#include "commands/slash/mog/cmd_data.h"

#include <dpp/message.h>
#include <dpp/coro/task.h>
#include <commands/slash/mog/command_type.h>
#include <commands/slash/mog/init_type_flag.h>

mln::mog::mog_help::mog_help(dpp::cluster& cluster) : base_mog_command{ cluster } {}

dpp::task<void> mln::mog::mog_help::command(const dpp::slashcommand_t& event_data, cmd_data_t& cmd_data, const command_type) const
{
    static const dpp::message s_info = dpp::message{ "Information regarding the `/mog` commands..." }
        .set_flags(dpp::m_ephemeral)
        .add_embed(dpp::embed{}.set_description(R"""(The `/mog` commands are utility tools for the game *Eminence in Shadow : Master of Garden*. The bot's database is used to store MOG teams.

The main MOG commands are:

- `/mog team`: A set of commands for managing MOG teams.
- `/mog arma`: A set of commands for managing cooldowns and other aspects of MOG's Armageddon event. These commands require at least one existing MOG team.

Each command set includes a `help` variant, which provides detailed information about the respective commands.)"""));

    co_await mln::response::co_respond(cmd_data.data, s_info, false, "Failed to reply with the mog help text!");
    co_return;
}

mln::mog::init_type_flag mln::mog::mog_help::get_requested_initialization_type(const command_type cmd) const
{
	return mln::mog::init_type_flag::none;
}

bool mln::mog::mog_help::is_db_initialized() const
{
	return true;
}
