#include "commands/base_action.h"
#include "commands/base_command.h"
#include "commands/ctx/base_ctx_command.h"

#include <dpp/appcommand.h>

#include <type_traits>

mln::base_ctx_command::base_ctx_command(dpp::cluster& cluster, dpp::slashcommand cmd) : base_command{ cluster, cmd } {}

mln::base_ctx_command::base_ctx_command(const base_ctx_command& rhs) : base_command{ rhs } {}

mln::base_ctx_command::base_ctx_command(base_ctx_command&& rhs) : base_command{ std::forward<base_ctx_command>(rhs) } {}

mln::base_ctx_command& mln::base_ctx_command::operator=(const base_ctx_command& rhs)
{
	base_command::operator=(rhs);

	return *this;
}

mln::base_ctx_command& mln::base_ctx_command::operator=(base_ctx_command&& rhs)
{
	base_command::operator=(std::forward<base_ctx_command>(rhs));

	return *this;
}
