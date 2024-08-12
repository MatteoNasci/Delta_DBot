#include "commands/ctx/base_ctx_command.h"
#include "bot_delta.h"

mln::base_ctx_command::base_ctx_command(mln::bot_delta* delta, dpp::slashcommand&& cmd) : base_command(delta, std::move(cmd)) {}

mln::base_ctx_command::base_ctx_command(base_ctx_command&& rhs) : base_command(std::move(rhs)) {}

mln::base_ctx_command& mln::base_ctx_command::operator=(base_ctx_command&& rhs){
	base_command::operator=(std::move(rhs));
	return *this;
}
