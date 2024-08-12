#include "commands/slash/base_slashcommand.h"
#include "bot_delta.h"

mln::base_slashcommand::base_slashcommand(mln::bot_delta* const delta, dpp::slashcommand&& cmd) : base_command(delta, std::move(cmd)) {}

mln::base_slashcommand::base_slashcommand(base_slashcommand&& rhs) : base_command(std::move(rhs)){}

mln::base_slashcommand& mln::base_slashcommand::operator=(base_slashcommand&& rhs){
	base_command::operator=(std::move(rhs));
	return *this;
}
