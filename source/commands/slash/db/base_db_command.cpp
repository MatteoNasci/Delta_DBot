#include "commands/base_action.h"
#include "commands/slash/db/base_db_command.h"
#include "commands/slash/db/db_cmd_data.h"
#include "commands/slash/db/db_command_type.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

mln::base_db_command::base_db_command(dpp::cluster& cluster) noexcept : base_action{ cluster } {}

bool mln::base_db_command::use_job() const noexcept
{
    return false;
}

void mln::base_db_command::job(const dpp::slashcommand_t&, db_cmd_data_t&, const db_command_type)
{
    cbot().log(dpp::loglevel::ll_critical, "Failed base db command! Usage of job command instead of task command.");
}
