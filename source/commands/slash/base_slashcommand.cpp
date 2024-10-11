#include "commands/base_command.h"
#include "commands/slash/base_slashcommand.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/misc-enum.h>

#include <format>
#include <type_traits>

mln::base_slashcommand::base_slashcommand(dpp::cluster& cluster, dpp::slashcommand&& cmd) : base_command{ cluster, std::forward<dpp::slashcommand>(cmd) } {}

void mln::base_slashcommand::log_incorrect_command() const
{
	cbot().log(dpp::loglevel::ll_critical, std::format("Failed [{}] command! Incorrect task/job usage.", get_cmd().name));
}
