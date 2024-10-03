#include "commands/base_action.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/cmd_data.h"
#include "commands/slash/mog/command_type.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

mln::mog::base_mog_command::base_mog_command(dpp::cluster& cluster) : base_action{ cluster } {}

bool mln::mog::base_mog_command::use_job() const
{
	return false;
}

void mln::mog::base_mog_command::job(const dpp::slashcommand_t&, mln::mog::cmd_data_t&, const mln::mog::command_type) const
{
	bot().log(dpp::loglevel::ll_critical, "Failed base mog command! Incorrect task/job usage.");
}
