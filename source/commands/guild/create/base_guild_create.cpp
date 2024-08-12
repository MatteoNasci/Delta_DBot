#include "commands/guild/create/base_guild_create.h"

mln::base_guild_create::base_guild_create(bot_delta* const delta) : base_action(delta){}

mln::base_guild_create::base_guild_create(base_guild_create&& rhs) : base_action(std::move(rhs)) {}

mln::base_guild_create& mln::base_guild_create::operator=(base_guild_create&& rhs) {
	base_action::operator=(std::move(rhs));
	return *this;
}
