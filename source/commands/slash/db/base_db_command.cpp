#include "commands/slash/db/base_db_command.h"

mln::base_db_command::base_db_command(dpp::cluster& cluster) : base_action{ cluster } {}