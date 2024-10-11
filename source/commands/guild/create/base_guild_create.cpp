#include "commands/base_action.h"
#include "commands/guild/create/base_guild_create.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>
#include <type_traits>

mln::base_guild_create::base_guild_create(dpp::cluster& cluster) noexcept : base_action{ cluster } {}

mln::base_guild_create::base_guild_create(const base_guild_create& rhs) noexcept : base_action{ rhs } {}

mln::base_guild_create::base_guild_create(base_guild_create&& rhs) noexcept : base_action{ std::forward<base_guild_create>(rhs) } {}

mln::base_guild_create& mln::base_guild_create::operator=(const base_guild_create& rhs) noexcept
{
	mln::base_action<dpp::task<void>, std::optional<std::function<void()>>, const dpp::guild_create_t&>::operator=(rhs);

	return *this;
}

mln::base_guild_create& mln::base_guild_create::operator=(base_guild_create&& rhs) noexcept
{
	mln::base_action<dpp::task<void>, std::optional<std::function<void()>>, const dpp::guild_create_t&>::operator=(rhs);

	return *this;
}
