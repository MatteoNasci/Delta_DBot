#include "commands/slash/base_slashcommand.h"

mln::base_slashcommand::base_slashcommand(dpp::cluster& cluster, dpp::slashcommand&& cmd) : base_command{ cluster, std::forward<dpp::slashcommand>(cmd) } {}