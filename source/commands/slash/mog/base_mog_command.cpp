#include "commands/base_action.h"
#include "commands/slash/mog/base_mog_command.h"
#include "commands/slash/mog/mog_cmd_data.h"
#include "commands/slash/mog/mog_command_type.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

mln::mog::base_mog_command::base_mog_command(dpp::cluster& cluster) : base_action{ cluster } {}

bool mln::mog::base_mog_command::use_job() const noexcept
{
	return false;
}

void mln::mog::base_mog_command::job(const dpp::slashcommand_t&, mln::mog::mog_cmd_data_t&, const mln::mog::mog_command_type)
{
	cbot().log(dpp::loglevel::ll_critical, "Failed base mog command! Incorrect task/job usage.");
}
