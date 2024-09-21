#include "commands/ctx/base_ctx_command.h"

mln::base_ctx_command::base_ctx_command(dpp::cluster& cluster, dpp::slashcommand&& cmd) : base_command{ cluster, std::forward<dpp::slashcommand>(cmd) } {}