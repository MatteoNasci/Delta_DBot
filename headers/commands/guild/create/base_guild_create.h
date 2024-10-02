#pragma once
#ifndef H_MLN_DB_BASE_GUILD_CREATE_H
#define H_MLN_DB_BASE_GUILD_CREATE_H

#include "commands/base_action.h"

#include <dpp/coro/task.h>

#include <functional>
#include <optional>

namespace dpp {
	class cluster;
	struct guild_create_t;
}

namespace mln {
	class base_guild_create : public base_action<dpp::task<void>, std::optional<std::function<void()>>, const dpp::guild_create_t&> {
	protected:
		base_guild_create(dpp::cluster& cluster);
	public:

		base_guild_create() = delete;

		base_guild_create(const base_guild_create&) = default;

		base_guild_create(base_guild_create&&) = default;

		base_guild_create& operator=(const base_guild_create&) = default;

		base_guild_create& operator=(base_guild_create&&) = default;
	};
}

#endif //H_MLN_DB_BASE_GUILD_CREATE_H